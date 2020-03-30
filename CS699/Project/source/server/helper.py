import subprocess
import constants


def validate_machine(lab_id, machine_id):
    """Validates whether the ip address belongs to the specified lab.

    :param lab_id: id of the lab, anyone of {sl1,sl2,cs101}
    :type lab_id: str
    :param machine_id: machine id is the last octet of the ip address
    :type machine_id: int
    :return:
    """
    if machine_id == constants.HEAD_NODE:
        return True
    elif machine_id <= 0:
        return False
    if lab_id == 'sl1':
        if machine_id <= constants.SL1_LAST_NODE:
            return True
    elif lab_id == 'sl2':
        if machine_id <= constants.SL2_LAST_NODE:
            return True
    elif lab_id == 'sl3':
        if machine_id <= constants.SL3_LAST_NODE:
            return True
    elif lab_id == 'cs101':
        if machine_id <= constants.CS101_LAST_NODE:
            return True
    return False


def get_ip_of(lab_id, machine_id):
    """Returns the ip address of the specified machine id of the specified lab.

    :param lab_id: id of the lab, anyone of {sl1,sl2,cs101}
    :type lab_id: str
    :param machine_id: machine id is the last octet of the ip address
    :type machine_id: int
    :return: ip_address (str)
    """
    if not validate_machine(lab_id, machine_id):
        raise ValueError('Invalid lab id or machine id')
    if lab_id == 'sl1':
        return constants.SL1_PREFIX + str(machine_id)
    elif lab_id == 'sl2':
        return constants.SL2_PREFIX + str(machine_id)
    elif lab_id == 'sl1':
        return constants.SL3_PREFIX + str(machine_id)
    elif lab_id == 'cs101':
        return constants.CS101_PREFIX + str(machine_id)


def get_lab_prefix(lab_id):
    """Returns the prefix part of the ip address of the specified lab, along with last ip octet of the last node.

    :param lab_id:
    :return: tuple (prefix,LAST_NODE)
    """
    if lab_id == 'sl1':
        return constants.SL1_PREFIX, constants.SL1_LAST_NODE
    elif lab_id == 'sl2':
        return constants.SL2_PREFIX, constants.SL2_LAST_NODE
    elif lab_id == 'sl3':
        return constants.SL3_PREFIX, constants.SL3_LAST_NODE
    elif lab_id == 'cs101':
        return constants.CS101_PREFIX, constants.CS101_LAST_NODE
    return None, None


def ping_lab(lab_id):
    """Pings the specified ip and returns the status.

    :param lab_id: id of the lab, anyone of {sl1,sl2,cs101}
    :type lab_id: str
    :return: bool that represents online status
    """
    if lab_id == 'sl1':
        return {ip: ping(ip) for ip in constants.SL1_IPs}
    elif lab_id == 'sl2':
        return {ip: ping(ip) for ip in constants.SL2_IPs}
    elif lab_id == 'sl3':
        return {ip: ping(ip) for ip in constants.SL3_IPs}
    elif lab_id == 'cs101':
        return {ip: ping(ip) for ip in constants.CS101_IPs}
    else:
        raise ValueError('Invalid Lab ID')


def ping(ip):
    """Pings the specified ip and returns the status.

    :param ip: ip address to be pinged
    :type ip: str
    :return: bool that represents online status
    """
    results = subprocess.call(['ping', ip, '-c1', '-W0.1'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    return True if results == 0 else False
