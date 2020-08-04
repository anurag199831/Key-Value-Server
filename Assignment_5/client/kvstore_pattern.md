### Get Value Request
        <?xml version="1.0" encoding="UTF-8"?>
        <Destination port="port">
        <KVMessage type="getreq">
        <Key>key</Key>
        </KVMessage>

### Put Value Request:
        <?xml version="1.0" encoding="UTF-8"?>
        <Destination port="port">
        <KVMessage type="putreq">
        <Key>key</Key>
        <Value>value</Value>
        </KVMessage>

### Delete Value Request:
        <?xml version="1.0" encoding="UTF-8"?>
        <Destination port="port">
        <KVMessage type="delreq">
        <Key>key</Key>
        </KVMessage>
