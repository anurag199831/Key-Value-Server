import csv

dictionary = dict()
with open('input0.csv', 'r') as infile:
    reader = csv.reader(infile)
    with open('outfile0.csv', 'w') as outfile:
        writer = csv.writer(outfile)
        for tokens in reader:
            if tokens[0].lower() == 'put':
                dictionary[tokens[1]] = tokens[2]
                writer.writerow(('Success',))
            elif tokens[0].lower() == 'get':
                if tokens[1] in dictionary.keys():
                    writer.writerow((tokens[1], dictionary[tokens[1]]))
                else:
                    writer.writerow(('Does not exist',))
            elif tokens[0].lower() == 'del':
                try:
                    del dictionary[tokens[1]]
                    writer.writerow(('Success',))
                except KeyError as e:
                    writer.writerow(('Does not exist',))
