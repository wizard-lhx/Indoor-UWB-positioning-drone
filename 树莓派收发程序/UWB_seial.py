import serial
import string
import binascii

s = serial.Serial(port="/dev/ttyUSB0",baudrate=115200)
s1= serial.Serial(port="/dev/ttyACM1",baudrate=500000)
mode = 1
step=1
data8_9 = 0
data11_12 = 0
data8 = 0
data9 = 0
data11 = 0
data12 = 0
if (mode == 1):
    d = bytes.fromhex('01 10 00 28 00 01 02 00 04 A1 BB')
    s.write(d)
else:
    d = bytes.fromhex('01 10 00 28 00 01 02 00 00 A0 78')
    s.write(d)
    #s.close()

'''if(s.isOpen == 0):
    s.open()
    print("send")'''



while True:

    data = str(binascii.b2a_hex(s.read()))[2:-1]    ##[2；-1]为字符串截取操作，表示每次截取两位
    #print(data)
    if (step == 1 and data == "01"):
        #print("step = ",step)
        step+=1
    elif (step == 2 and data == "03"): 
        #print("step = ",step)
        step+=1
    elif (step == 2 and data == "10"):
        step = 1
    elif (step == 2 and data == "81"):
        step = 1
    elif (step >= 3):
        '''if (step == 7):
            station = int(data,16)
            if (station >= 1 and station <= 8 ):
                print("station = ",station)
                step+=1
                print(step)
            elif (station == 0 and data == 00):
                print("wrong station step")'''
                
        if (step == 8):
            data8 = int(data,16)
            data9 = int(str(binascii.b2a_hex(s.read()))[2:-1],16)
            #print("step = ",step)
            print("data8 = ",data8)
            print("data9 = ",data9)
            '''
            print("data9 = ",data9)
            data8_9 = data+data9
            data8 = hex(data)
            data9 = hex(data9)
            print("data8_9 = ",data8_9)
            print("distance_hex = ",data8_9)
            distance_x = int(data8_9,16)
            
            print("distance_x = ",distance_x)
            '''
            step+=2
            
        if (step == 11):
            data11 = int(data,16)
            data12 = int(str(binascii.b2a_hex(s.read()))[2:-1],16)
            #print("step = ",step)
            print("data11 = ",data11)
            print("data12 = ",data12)
            '''
            data11_12 = data+data12
            data11 = hex(data)
            data12 = hex(data12)
            
            print("distance_hex = ",data11_12)
            distance_y = int(data11_12,16)
            print("distance_y = ",distance_y)
            '''
            step+=2
            
        elif (step == 45):
            #print("step = ",step)
            step = 1
        else:
            #print("step = ",step)
            step+=1
            
           
    buf = bytearray([0xDA,0xDA,data8,data9,data11,data12,0xBC])
    s1.write(buf)

