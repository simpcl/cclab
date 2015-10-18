
import sys,os
import tfstool

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print 'invalid args'
        sys.exit(-1)

    file_path = sys.argv[1]
    file_size = 0
    try:
        file_size = os.path.getsize(file_path)
        f = open(file_path, 'rb')
        crc = 0
        if file_size <= 4096:
            data = f.read()
            crc = tfstool.crc32(data)
        else:
            while True:
                data = f.read(4096)
                if len(data) == 0:
                    break
                crc = tfstool.crc32(data, crc)
        f.close()
        print crc
    except Exception,e:
        print 'Exception', e
