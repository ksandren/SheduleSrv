#imports***********************************************************************
import MySQLdb
#******************************************************************************

#globals***********************************************************************
defaults = {'host': 'localhost', 'user': 'tt', 'passwd': 'cern99', 'db': 'tt'}
db = MySQLdb.connect(**defaults)
c=db.cursor()
#******************************************************************************

#main function*****************************************************************
def application(environ, start_response):
    start_response('200 OK', [ ('Content-type', 'text/xml') ])
    tables = ['vinf', 'defaults', 'shedule_mode', 'param', 'value']
    yield b'<?xml version="1.0" encoding="UTF-8"?>'
    yield b'<x>'
    for table in tables:
        result = b'<table name="' + table.encode('utf-8') + b'">'
        params = getTable(table)
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
    yield b'</x>'
    c.close()
    db.close()
#******************************************************************************

def getTable(table):
    c.execute("SELECT * FROM `" + table + "`")
    return c