#imports***********************************************************************
import MySQLdb
import re
#******************************************************************************

#globals***********************************************************************
defaults = {'host': 'localhost', 'user': 'tt', 'passwd': 'cern99', 'db': 'tt'}
db = MySQLdb.connect(**defaults)
c=db.cursor()
#******************************************************************************

#main function*****************************************************************
def application(environ, start_response):
    try:
        request_body_size = int(environ.get('CONTENT_LENGTH', 0))
    except (ValueError):
        request_body_size = 0

    start_response('200 OK', [ ('Content-type', 'text/xml') ])
    yield b'<?xml version="1.0" encoding="UTF-8"?>'
    yield b'<x>'
    if request_body_size == 0:
        for msg in send_basic_data():
            yield msg
    else:
        for msg in send_shedule_data(environ, request_body_size):
            yield msg
    yield b'</x>'
    #c.close()
    db.close()
#******************************************************************************
def send_basic_data():
    tables = ['vinf', 'defaults', 'shedule_mode', 'param', 'value']
    for table in tables:
        result = b'<table name="' + table.encode('utf-8') + b'">'
        params = get_table(table)
        for record in params:
            s = '<r>'
            for value in record:
                s += '<v>' + str(value) + '</v>'
            result += (s.encode('utf-8') + b'</r>')
            if len(result) > 16 * 1024:
               yield result
               result = b'' 
        result += b'</table>'
        yield result


def get_table(table):
    c.execute("SELECT * FROM `" + table + "`")
    return c


def send_shedule_data(environ, request_body_size):
    request_body = environ['wsgi.input'].read(request_body_size).decode('utf-8')
    mode = re.search(r'mode=(\d+)', request_body)
    try:
        if(mode != None):
            mode = int(mode.group(1))
    except (ValueError):
        mode = None
    attr = re.findall(r'(\d+)=(\d+)', request_body)
    if(len(attr) == 0 or mode == None):
        yield b'ERROR'
        return [b'']
    c.execute('SELECT `id`, `name` FROM `param` WHERE `mode_id` = ' + str(mode))
    params = {}
    for record in c:
        params[str(record[0])] = str(record[1])
    fields = '`'
    for it in attr:
        fields += params[it[0]] + '`=' + it[1] + ' AND `'
    fields = fields[:-6]
    result = ('<table name="mode_' + str(mode) + '_session">').encode('utf-8')
    c.execute('SELECT * FROM `mode_' + str(mode) + '_session_mirror` WHERE ' + fields)
    for record in c:
        s = '<r>'
        for value in record:
            s += '<v>' + str(value) + '</v>'
        result += (s.encode('utf-8') + b'</r>')
        if len(result) > 16 * 1024:
           yield result
           result = b'' 
    result += b'</table>'
    yield result
