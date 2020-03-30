

import getpass
import socket
import paramiko
from scp import SCPClient
from flask import Flask, jsonify

import constants
import helper

app = Flask(__name__)
macs = {}
user = None
passwd = None


@app.route('/lab/<string:lab_id>/<int:machine_id>/cpu_stat', methods=['GET'])
def cpu_stats(lab_id, machine_id):
    """Returns the cpu statistics per core along with the average cpu utilisation.

    :param lab_id: id of the lab, anyone of {sl1,sl2,cs101}
    :type lab_id: str
    :param machine_id: machine id is the last octet of the ip address
    :type machine_id: int
    :return: json containing the cpu statistics
    """
    def get_usage(before, after):
        busy_diff = after[0] - before[0]
        idle_diff = after[1] - before[1]
        try:
            usage = busy_diff / (busy_diff + idle_diff)
        except ZeroDivisionError:
            usage = 0.0
        return usage * 100

    def get_time_profile(time):
        idle_time = time[3] + time[4]
        busy_time = sum(time[:-2]) - idle_time
        return busy_time, idle_time

    ip = helper.get_ip_of(lab_id, machine_id)
    command = 'cat /proc/stat | awk \'{if(NF == 11) print $0}\'&& sleep 1 && cat /proc/stat | awk \'{if(NF == 11) ' \
              'print $0}\' '

    # Oopen a connection with Cient
    ssh_client = paramiko.SSHClient()
    ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        ssh_client.connect(ip, username=user, password=passwd, timeout=2)
        _, stdout, stderr = ssh_client.exec_command(command, timeout=2)
        output = stdout.readlines()
    except socket.timeout:
        output = {}
    finally:
        ssh_client.close()
    if len(output) % 2 == 0:
        divider = len(output) // 2
    else:
        # Got incorrect response from the client. Send an empty json
        return jsonify({})
    stat_before = {line[0]: list(map(int, line[1:]))
                   for line in [l.split() for l in output[:divider]]}
    stat_after = {line[0]: list(map(int, line[1:]))
                  for line in [l.split() for l in output[divider:]]}
    ret_val = dict()
    for key in stat_before.keys():
        ret_val[key] = get_usage(get_time_profile(stat_before[key]), get_time_profile(stat_after[key]))
    return jsonify(ret_val)


@app.route('/lab/<string:lab_id>/<int:machine_id>/hardware', methods=['GET'])
def machine_hardware_info(lab_id, machine_id):
    """Returns whether the specified machine has keyboard and hardware attached to it.

    :param lab_id: id of the lab, anyone of {sl1,sl2,cs101}
    :type lab_id: str
    :param machine_id: machine id is the last octet of the ip address
    :type machine_id: int
    :return: json of type {'keyboard': bool,'mouse':bool}
    """
    ip = helper.get_ip_of(lab_id, machine_id)
    command = 'sudo lsusb -v| grep -i iproduct'
    output = str()
    #Open up a connection with Client
    ssh_client = paramiko.SSHClient()
    ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        ssh_client.connect(ip, username=user,
                           password=passwd, timeout=2)
        channel = ssh_client.invoke_shell()
        command_flag = True
        passwd_flag = True
        prompt_flag = True
        while True:
            if channel.recv_ready():
                output += channel.recv(9999).decode('utf-8')
            elif prompt_flag:
                continue
            if output.endswith('[sudo] password for {}: '.format(user)) and passwd_flag:
                # Enter the password for sudo
                channel.send(passwd + '\n')
                passwd_flag = False
                output = str()
            elif output.endswith('{}@{}-{}:~$ '.format(user, lab_id, machine_id)):
                # Wait for Prompt
                if prompt_flag:
                    output = str()
                    prompt_flag = False
                elif not command_flag:
                    break
            elif command_flag and not prompt_flag:
                channel.send(command + '\n')
                command_flag = False
    except socket.timeout:
        # Empty json on error
        return jsonify({})
    finally:
        ssh_client.close()
    output = output.lower()
    ret_val = {'keyboard': 'keyboard' in output, 'mouse': 'mouse' in output}
    return jsonify(ret_val)


@app.route('/lab/<string:lab_id>/hardware/<string:hardware_name>', methods=['GET'])
def lab_hardware_info(lab_id, hardware_name):
    """Returns the specified hardware over all the specified lab machines.

    :param lab_id: id of the lab, anyone of {sl1,sl2,cs101}
    :type lab_id: str
    :param hardware_name: hardware_name can be any of the {keyboard, mouse}
    :type hardware_name: str
    :return: json containing the associated boolean values over all lab machines
    """
    if hardware_name not in ['keyboard', 'mouse']:
        return jsonify({})
    command = 'sudo lsusb -v| grep -i iproduct'
    lab_prefix, last_node = helper.get_lab_prefix(lab_id)
    if lab_prefix is None:
        raise ValueError('Invalid lab id')
    status = helper.ping_lab(lab_id)
    ret_val = {}
    ips = list(range(1, last_node + 1))
    ips.append(constants.HEAD_NODE)
    for i in ips:
        output = str()
        ip = lab_prefix + str(i)
        if not status[ip]:
            ret_val[ip] = False
            continue
        # Open up a connection with the client
        ssh_client = paramiko.SSHClient()
        ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        try:
            ssh_client.connect(ip, username=user,
                               password=passwd, timeout=2)
            print(ip)
            channel = ssh_client.invoke_shell()
            command_flag = True
            passwd_flag = True
            prompt_flag = True
            fail_flag = False
            while True:
                if channel.recv_ready():
                    output += channel.recv(9999).decode('utf-8')
                elif prompt_flag:
                    continue
                if output.endswith('[sudo] password for {}: '.format(user)) and passwd_flag:
                    # Enter password on sudo prompt
                    channel.send(passwd + '\n')
                    passwd_flag = False
                    output = str()
                elif output.endswith('{}@{}-{}:~$ '.format(user, lab_id, i)):
                    # Wait for the next shell prompt
                    if prompt_flag:
                        output = str()
                        prompt_flag = False
                    elif not command_flag:
                        break
                elif command_flag and not prompt_flag:
                    channel.send(command + '\n')
                    command_flag = False
        except (socket.timeout, paramiko.SSHException, paramiko.ssh_exception.SSHException) as e:
            ret_val[ip] = False
            fail_flag = True
        finally:
            ssh_client.close()
        if not fail_flag:
            ret_val[ip] = hardware_name in output.lower()
    return jsonify(ret_val)


@app.route('/lab/<string:lab_id>/status', methods=['GET'])
def lab_status(lab_id):
    """Return all the online status of all the machines of the specified lab id.

    :param lab_id: id of the lab, anyone of {sl1,sl2,cs101}
    :type lab_id: str
    :return: json containing associated bool values over every lab machine
    """
    return jsonify(helper.ping_lab(lab_id))


@app.route('/lab/<string:lab_id>/<int:machine_id>/status', methods=['GET'])
def lab_machine_status(lab_id, machine_id):
    """Returns the online status of the specified machine in the specified lab.

    :param lab_id: id of the lab, anyone of {sl1,sl2,cs101}
    :type lab_id: str
    :param machine_id: machine id is the last octet of the ip address
    :type machine_id: int
    :return: json containing the bool value associated with the status of the machine
    """
    ip = helper.get_ip_of(lab_id, machine_id)
    return jsonify({ip: helper.ping(ip)})


@app.route('/lab/<string:lab_id>/shutdown', methods=['GET'])
def shutdown(lab_id):
    """Turn off all the machines in the specified lab.

    :param lab_id: id of the lab, anyone of {sl1,sl2,cs101}
    :type lab_id: str
    :return: None
    """
    status = helper.ping_lab(lab_id)
    print(status)
    command = 'sudo shutdown now'
    # Opening connection with the client
    ssh_client = paramiko.SSHClient()
    ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    prefix, last_node = helper.get_lab_prefix(lab_id)

    for i in range(1, last_node + 1):
        ip = helper.get_ip_of(lab_id, i)
        if not status[ip]:
            continue
        try:
            print('Connecting to {}'.format(ip))
            ssh_client.connect(ip, username=user,
                               password=passwd, timeout=2)
            # Invoking shell as  it is an interactive session
            channel = ssh_client.invoke_shell()
            print('Shutting down {}'.format(ip))
            prompt_flag = True
            output = str()
            while True:
                if channel.recv_ready():
                    output += channel.recv(9999).decode('utf-8')
                elif prompt_flag:
                    continue
                if output.endswith('[sudo] password for {}: '.format(user)):
                    # Input password on sudo prompt
                    channel.send(passwd + '\n')
                    break
                elif output.endswith('{}@{}-{}:~$ '.format(user, lab_id, i)):
                    # Wait for the next shell prompt
                    channel.send(command + '\n')
                    prompt_flag = False
        except (
                socket.timeout, paramiko.ssh_exception.SSHException,
                paramiko.ssh_exception.NoValidConnectionsError) as e:
            print(e)
        finally:
            ssh_client.close()
            print('Disconnected from {}'.format(ip))

    return jsonify({})


@app.route('/lab/<string:lab_id>/wakeup', methods=['GET'])
def wakeup(lab_id):
    """Turn on all the machines in the specified lab.

    :param lab_id: id of the lab, anyone of {sl1,sl2,cs101}
    :type lab_id: str
    :return: None
    """
    ip = helper.get_ip_of(lab_id, constants.HEAD_NODE)
    hw_ids = macs[lab_id].values()
    hw_ids = ' '.join(hw_ids)
    command = 'parallel -u \'wakeonlan\' ::: {}'.format(hw_ids)
    ssh_client = paramiko.SSHClient()
    ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        ssh_client.connect(ip, username=user, password=passwd, timeout=2)
        # Non interactive client, hence using exec_command
        _, stdout, stderr = ssh_client.exec_command(command, timeout=2)
    except socket.timeout:
        pass
    finally:
        ssh_client.close()
    return jsonify({})


@app.route('/lab/<string:lab_id>/<int:machine_id>/software', methods=['GET'])
def list_of_softwares(lab_id, machine_id):
    """Returns the list of softwares installed on the system specified in the given lab.

    :param lab_id: id of the lab, anyone of {sl1,sl2,cs101}
    :type lab_id: str
    :param machine_id: machine id is the last octet of the ip address
    :type machine_id: int
    :return: json containing the list of softwares
    """
    ip = helper.get_ip_of(lab_id, machine_id)
    command = 'apt-mark showmanual'
    output = {}
    # Open up the ssh session with the client
    ssh_client = paramiko.SSHClient()
    ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        ssh_client.connect(ip, username=user, password=passwd, timeout=2)
        # Non interactive client, hence using exec_command
        _, stdout, stderr = ssh_client.exec_command(command, timeout=2)
        output = stdout.readlines()
    except socket.timeout:
        output = {}
    finally:
        ssh_client.close()
    output = [i.split('\n')[0] for i in output]
    return jsonify({'installed': output})


@app.route('/lab/<string:lab_id>/update_macs', methods=['GET'])
def update_mac_to_ip_mapping(lab_id):
    """Gets the mac id to ip mapppings of all the machines in the specified lab.

    :param lab_id: id of the lab, anyone of {sl1,sl2,cs101}
    :type lab_id: str
    :return: json containing the mapping
    """
    prefix, last_node = helper.get_lab_prefix(lab_id)
    node = prefix + str(constants.HEAD_NODE)
    output = str()
    ssh_client = paramiko.SSHClient()
    ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    try:
        ssh_client.connect(node, username=user,
                           password=passwd, timeout=2)
        channel = ssh_client.invoke_shell()
        command_flag = True
        prompt_flag = True
        flush_flag = True
        ping_flag = True
        while True:
            if channel.recv_ready():
                output += channel.recv(9999).decode('utf-8')
            elif prompt_flag:
                continue
            if output.endswith('[sudo] password for {}: '.format(user)):
                channel.send(passwd + '\n')
                output = str()
            elif output.endswith('{}@{}-{}:~$ '.format(user, lab_id, constants.HEAD_NODE)):
                if prompt_flag:
                    output = str()
                    prompt_flag = False
                elif not command_flag and not ping_flag and not flush_flag:
                    break
            if flush_flag and not prompt_flag:
                channel.send('sudo ip neigh flush all' + '\n')
                flush_flag = False
                prompt_flag = True
            elif ping_flag and not prompt_flag:
                channel.send(
                    'parallel -u \'ping -c1 -W1\' ::: {}{{1..{}}} 2>/dev/null 1>/dev/null'.format(prefix,
                                                                                                  last_node) + '\n')
                ping_flag = False
                prompt_flag = True
            elif command_flag and not prompt_flag:
                channel.send(
                    'arp -e | awk \'{ if(length($3)==17) {print $1,$3 }}\'' + '\n')
                command_flag = False
                output = str()
    except socket.timeout:
        pass
    finally:
        ssh_client.close()
    if output is None or len(output) == 0:
        return jsonify({})
    output = output.splitlines()[1:-1]
    output = [x.split() for x in output]
    output = {x[0]: x[1] for x in output}
    macs[lab_id] = output
    return jsonify({})


@app.route('/lab/<string:lab_id>/exam_mode_on', methods=['GET'])
def exam_mode_on(lab_id):
    """Turns on the exam mode for the specified lab which disables the usb port for memory devices.

    :param lab_id: id of the lab, anyone of {sl1,sl2,cs101}
    :type lab_id: str
    :return: None
    """
    status = helper.ping_lab(lab_id)
    command = './on-script.sh'
    chmod_command = 'chmod +x on-script.sh'
    ssh_client = paramiko.SSHClient()
    ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    prefix, last_node = helper.get_lab_prefix(lab_id)

    for i in range(1, last_node):
        ip = prefix+str(i)
        if not status[ip]:
            continue
        try:
            print('Connecting to {}'.format(ip))
            ssh_client.connect(ip, username=user,
                               password=passwd, timeout=2)
            # Sending file to the client
            with SCPClient(ssh_client.get_transport()) as scp:
                scp.put('on-script.sh', 'on-script.sh')
            # Invoke shell since it is an interactive client
            channel = ssh_client.invoke_shell()
            passwd_flag = True
            prompt_flag = True
            chmod_flag = True
            exec_flag = False
            output = str()
            while True:
                if channel.recv_ready():
                    output += channel.recv(9999).decode('utf-8')
                elif prompt_flag:
                    continue
                if output.endswith('[sudo] password for {}: '.format(user)) and passwd_flag:
                    # Sending Password on prompt
                    passwd_flag = False
                    channel.send(passwd + '\n')
                elif output.endswith('{}@{}-{}:~$ '.format(user, lab_id, i)) and chmod_flag:
                    # Giving execution permissions to the sent script
                    channel.send(chmod_command + '\n')
                    prompt_flag = False
                    chmod_flag = False
                    exec_flag = True
                elif output.endswith('{}@{}-{}:~$ '.format(user, lab_id, i)) and exec_flag:
                    # Sending command to turn on the exam mode
                    channel.send(command + '\n')
                    exec_flag = False
                elif output.endswith('{}@{}-{}:~$ '.format(user, lab_id, i)) and not passwd_flag:
                    break
            # print(output)
        except (
                socket.timeout, paramiko.ssh_exception.SSHException,
                paramiko.ssh_exception.NoValidConnectionsError) as e:
            print(e)
        finally:
            ssh_client.close()
    return jsonify({})


@app.route('/lab/<string:lab_id>/exam_mode_off', methods=['GET'])
def exam_mode_off(lab_id):
    """Turns off the exam mode for the specified lab which enables the usb port for memory devices.

    :param lab_id: id of the lab, anyone of {sl1,sl2,cs101}
    :type lab_id: str
    :return: None
    """
    status = helper.ping_lab(lab_id)
    command = './off-script.sh'
    chmod_command = 'chmod +x off-script.sh'
    ssh_client = paramiko.SSHClient()
    ssh_client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    prefix, last_node = helper.get_lab_prefix(lab_id)

    for i in range(1, last_node):
        ip = prefix+str(i)
        if not status[ip]:
            continue
        try:
            print('Connecting to {}'.format(ip))
            ssh_client.connect(ip, username=user,
                               password=passwd, timeout=2)
            # Sending file to the client
            with SCPClient(ssh_client.get_transport()) as scp:
                scp.put('off-script.sh', 'off-script.sh')
            # Invoke shell since it is an interactive client
            channel = ssh_client.invoke_shell()
            passwd_flag = True
            prompt_flag = True
            chmod_flag = True
            exec_flag = False
            output = str()
            while True:
                if channel.recv_ready():
                    output += channel.recv(9999).decode('utf-8')
                elif prompt_flag:
                    continue
                if output.endswith('[sudo] password for {}: '.format(user)) and passwd_flag:
                    # Sending Password on prompt
                    passwd_flag = False
                    channel.send(passwd + '\n')
                elif output.endswith('{}@{}-{}:~$ '.format(user, lab_id, i)) and chmod_flag:
                    # Giving execution permissions to the sent script
                    channel.send(chmod_command + '\n')
                    prompt_flag = False
                    chmod_flag = False
                    exec_flag = True
                elif output.endswith('{}@{}-{}:~$ '.format(user, lab_id, i)) and exec_flag:
                    # Sending command to turn off the exam mode
                    channel.send(command + '\n')
                    exec_flag = False
                elif output.endswith('{}@{}-{}:~$ '.format(user, lab_id, i)) and not passwd_flag:
                    break
            # print(output)
        except (
                socket.timeout, paramiko.ssh_exception.SSHException,
                paramiko.ssh_exception.NoValidConnectionsError) as e:
            print(e)
        finally:
            ssh_client.close()
    return jsonify({})


if __name__ == '__main__':
    with app.app_context():
        print('Enter the username: ', end='')
        user = input()
        passwd = getpass.getpass()
        # Update ip to mac mappings at the beginning of the server.
        update_mac_to_ip_mapping('sl1')
        update_mac_to_ip_mapping('sl2')
        update_mac_to_ip_mapping('sl3')
        update_mac_to_ip_mapping('cs101')
    app.run(host='0.0.0.0', port=5000)
