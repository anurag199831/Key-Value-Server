import csv
import random

funcs = ['PUT', 'GET', 'DEL']
keys = list()

for j in range(1):
    with open("input" + str(j) + ".csv", 'w') as file:
        writer = csv.writer(file)
        for i in range(10000):
            func = random.choice(funcs)
            if(func.lower() == 'get' or func.lower() == 'del'):
                try:
                    key = random.choice(keys)
                except IndexError as e:
                    key = random.randint(0, 10000)
                writer.writerow((func, key))
            else:
                key = random.randint(0, 10000)
                keys.append(key)
                value = random.randint(0, 10000)
                writer.writerow((func, key, value))
