import cv2
import urllib
import socket
import sdl2.ext
import numpy as np
from sdl2 import *
from sdl2.sdlttf import *
from threading import Thread, Lock
from ctypes import pointer, c_long
from time import sleep, strftime

displayInstructions = False
flashLightOn = False
HUDOn = True
done = False
rec = " "
resized = None

class Data:
    fontSize = 32
    fontPath = ""
    fontColor = None
    text = ""
    x = 0
    y = 0




# renderTexture, renderText, and other SDL_TTF code based on
# https://egdev.wordpress.com/2014/03/14/python-sdl2-ttf-test/
def renderTexture(texture, render, xPos, yPos):
    dest = SDL_Rect(xPos, yPos)
    w = pointer(c_long(0))
    h = pointer(c_long(0))
    SDL_QueryTexture(texture, None, None, w, h)
    dest.w = w.contents.value
    dest.h = h.contents.value
    SDL_RenderCopy(render, texture, None, dest)
    return


def renderText(message, fontFile, fontColor, fontSize, render):
    SDL_ClearError()
    font = TTF_OpenFont(fontFile, fontSize)
    p = SDL_GetError()
    
    if font is None or p != '':
        print("TTF_OPENFONT error: " + p)
        return None
    
    surf = TTF_RenderText_Blended(font, message, fontColor)
    if surf is None:
        TTF_CloseFont(font)
        print("TTF_RenderText error")
        return None
    
    txt = SDL_CreateTextureFromSurface(render, surf)
    if txt is None:
        print("CreateTexture error")

    SDL_FreeSurface(surf)
    TTF_CloseFont(font)
    return txt


def handleWASD(keyStatus):
    if keyStatus[SDL_SCANCODE_S] and keyStatus[SDL_SCANCODE_D]:
        return '8'
    elif keyStatus[SDL_SCANCODE_S] and keyStatus[SDL_SCANCODE_A]:
        return '7'
    elif keyStatus[SDL_SCANCODE_W] and keyStatus[SDL_SCANCODE_D]:
        return '6'
    elif keyStatus[SDL_SCANCODE_W] and keyStatus[SDL_SCANCODE_A]:
        return '5'
    elif keyStatus[SDL_SCANCODE_D]:
        return '4'
    elif keyStatus[SDL_SCANCODE_A]:
        return '3'
    elif keyStatus[SDL_SCANCODE_S]:
        return '2'
    elif keyStatus[SDL_SCANCODE_W]:
        return '1'
    
    return '0'


def handleArrowKeys(keyStatus):
    if keyStatus[SDL_SCANCODE_DOWN] and keyStatus[SDL_SCANCODE_RIGHT]:
        return '8'
    elif keyStatus[SDL_SCANCODE_DOWN] and keyStatus[SDL_SCANCODE_LEFT]:
        return '7'
    elif keyStatus[SDL_SCANCODE_UP] and keyStatus[SDL_SCANCODE_RIGHT]:
        return '6'
    elif keyStatus[SDL_SCANCODE_UP] and keyStatus[SDL_SCANCODE_LEFT]:
        return '5'
    elif keyStatus[SDL_SCANCODE_RIGHT]:
        return '4'
    elif keyStatus[SDL_SCANCODE_LEFT]:
        return '3'
    elif keyStatus[SDL_SCANCODE_DOWN]:
        return '2'
    elif keyStatus[SDL_SCANCODE_UP]:
        return '1'
        
    return '0'


def handleFlashLight(keyStatus):
    global flashLightOn
    
    if keyStatus[SDL_SCANCODE_F]:
        flashLightOn = not flashLightOn
        sleep(0.2)# Delay to simulate key press
        
    if flashLightOn:
        return 'F'
    
    return '0'

def handleUpDownPWM(keyStatus):
    if keyStatus[SDL_SCANCODE_E]:# Increase
        return 'U'
    elif keyStatus[SDL_SCANCODE_C]:# Decrease
        return 'D'
    
    return '0'

def handleKeyboardState(keyStatus):
    global displayInstructions
    global HUDOn
    global done

    message = ""
    # Car movement
    message += handleWASD(keyStatus)

    # Flashlight
    message += handleFlashLight(keyStatus)

    # Camera tilt
    message += handleArrowKeys(keyStatus)

    # Modify DC Motor PWM
    message += handleUpDownPWM(keyStatus)

    # Toggle Help
    if keyStatus[SDL_SCANCODE_H]:
        displayInstructions = not displayInstructions
        sleep(0.2)

    # Toggle HUD
    if keyStatus[SDL_SCANCODE_T]:
        HUDOn = not HUDOn
        sleep(0.2)
    
    # Quit
    if keyStatus[SDL_SCANCODE_ESCAPE]:
        done = True
        message = "Q"
        
    return message
        

def handleSerial():# Threaded
    global done
    global rec # Received data from Raspberry Pi

    while not done:
        sleep(0.05)
        sdl2.ext.get_events() # Have to poll SDL_Events to use GetKeyboardState
        
        keyStatus = SDL_GetKeyboardState(None)
        message = handleKeyboardState(keyStatus)
        
        # Send keyboard data
        # Receive sensor data as one string
        conn.send(message.encode())
        rec = str(conn.recv(1024)).translate(None, "\r\n")
    return


def createData(x, y, fontSize, fontPath, fontColor):
    temp = Data()
    temp.x = x
    temp.y = y
    temp.fontSize = fontSize
    temp.fontPath = fontPath
    temp.fontColor = fontColor
    temp.text = ""
    return temp


def writeDataToScreen(data, renderer):
    texture = renderText(data.text, data.fontPath, data.fontColor,
                                          data.fontSize, renderer)
    renderTexture(texture, renderer, data.x, data.y)
    SDL_DestroyTexture(texture)
    return


def createDataVals():
    dataVals = []
    color = SDL_Color(0, 255, 0)
    fontSize = 25
    dataVals.append(createData(50, 50, fontSize, fontPath, color))#0 Time
    dataVals.append(createData(520, 50, fontSize, fontPath, color))#1 Temp
    dataVals.append(createData(50, 400, fontSize, fontPath, color))#2 Light
    dataVals.append(createData(520, 400, fontSize, fontPath, color))#3 Distance
    dataVals.append(createData(290, 400, fontSize, fontPath, color))#4 DC PWM

    
    dataVals.append(createData(10, 460, 20, fontPath, color))#N-1 Help
    dataVals[len(dataVals) - 1].text = "H for Help"
    return dataVals


def createDataHelps():
    dataHelps = []
    color = SDL_Color(0, 200, 0)
    fontSize = 25
    
    # Data labels
    dataHelps.append(createData(50, 80, fontSize, fontPath, color))#0 Time help
    dataHelps.append(createData(520, 80, fontSize, fontPath, color))#1 Temp help
    dataHelps.append(createData(50, 430, fontSize, fontPath, color))#2 Light help
    dataHelps.append(createData(520, 430, fontSize, fontPath, color))#3 Distance help
    dataHelps.append(createData(290, 430, fontSize, fontPath, color))#4 PWM help

    # Instructions
    dataHelps.append(createData(100, 200, fontSize, fontPath, color)) #5 Car Movement
    dataHelps.append(createData(100, 240, fontSize, fontPath, color)) #6 Camera
    dataHelps.append(createData(100, 280, fontSize, fontPath, color)) #7 Flashlight
    dataHelps.append(createData(100, 320, fontSize, fontPath, color)) #8 DC PWM
    

    dataHelps[0].text = "Time"
    dataHelps[1].text = "Temp"
    dataHelps[2].text = "Light"
    dataHelps[3].text = "Distance"
    dataHelps[4].text = "Motor Speed"
    dataHelps[5].text = "WASD to move car"
    dataHelps[6].text = "Arrow keys to move camera"
    dataHelps[7].text = "F to toggle flashlight"
    dataHelps[8].text = "E to increase, C to decrease motor speed"
    return dataHelps




def main():
    global fontPath
    global conn
    
    windowSize = (640, 480)
    sdl2.ext.init()
    TTF_Init()

    fontPath = ("C:\\Users\\7216042\\Desktop\\Coding Programs\\ProjectCode\\"
                "Xerox Sans Serif Narrow Oblique.ttf")

    # Create data and help variables and put into arrays
    dataVals = createDataVals()
    dataHelps = createDataHelps()

    # These can't be grouped together in either dataHelps or dataVals
    # These will be shown on the screen at all times
    toggleHud = createData(470, 460, 20, fontPath, SDL_Color(0, 255, 0))
    toggleHud.text = "Press T to toggle HUD"
    serialConnecting = createData(80, 240, 25, fontPath, SDL_Color(0, 255, 0))
    serialConnecting.text = ""
    
    # Acts like a C/C++ enum
    TIME, TEMP, LIGHT, DISTANCE, PWM = list(range(0, 5))# range->[0, 5)

    
    HOST = '0.0.0.0'
    PORT = 5005
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((HOST, PORT))

    print('Listening for connection!')
    s.listen(1) # Waits for connection to be made with Rasp Pi
    s.setblocking(True)
    conn, addr = s.accept()

    # Need an OpenCV window open to use cv2 functions
    # Do not close the window generated or else keyboard input fails
    blank = np.zeros((3, 3, 3), np.uint8)
    cv2.imshow(' ', blank)

    # SDL / SDL_TTF / SDL_Window set up
    window = SDL_CreateWindow("EnviroBot", SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED, 640, 480,
                                   SDL_WINDOW_SHOWN)
    
    windowSurf = SDL_GetWindowSurface(window)
    windowArray = sdl2.ext.pixels3d(windowSurf.contents)

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED)
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND)

    stream = urllib.urlopen('http://192.168.1.103:8080/?action=stream')

    s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

    print 'Connected by', addr

    SerialThread = Thread(target=handleSerial, args=())
    SerialThread.start()
    bytes = ""
    
    while not done:
        # We use M-JPG Streamer to stream video over wifi
        # The following code is for reading the stream data
        # Read in video frame from the stream
        bytes += stream.read(2048)
        a = bytes.find('\xff\xd8')
        b = bytes.find('\xff\xd9')
        
        if a != -1 and b != -1:
            # Modify stream byte array to cv2 mat and resize
            # M-jpg Streamer code based on
            # http://petrkout.com/electronics/low-latency-0-4-s-video
            #      -streaming-from-raspberry-pi-mjpeg-streamer-opencv/
            jpg = bytes[a:b+2]
            bytes = bytes[b+2:]
            i = cv2.imdecode(np.fromstring(jpg, dtype=np.uint8),1)
            r = 720.0 / i.shape[1]
            dim = (640, int(i.shape[0] * r))
            resized = cv2.resize(i, dim, interpolation = cv2.INTER_AREA)
            
            # Modify cvMat to SDL2 Surface array
            # cvMat to SDL2 Surface code base on
            # http://stackoverflow.com/questions/18434348/converting-cv2
            #      -images-to-pysdl2-surfaces-for-blitting-to-screen
            ima = np.insert(resized, 3, 255, axis=2)
            ima = np.rot90(ima)
            np.copyto(windowArray, ima)

            # Split data fields to write to the screen
            # Example rec input: "23C/54F, Etc: 53"
            splitVals = rec.split(',')

            # Only output if the Pi has connected to Arduino
            if len(splitVals) >= 3:
                dataVals[TIME].text = strftime("%I:%M:%S%p")
                dataVals[TEMP].text = splitVals[0]
                dataVals[LIGHT].text = splitVals[1]
                dataVals[DISTANCE].text = splitVals[2]
                dataVals[PWM].text = splitVals[3]
                serialConnecting.text = ""
            else:
                dataVals[TIME].text = ""
                dataVals[TEMP].text = ""
                dataVals[LIGHT].text = ""
                dataVals[DISTANCE].text = ""
                dataVals[PWM].text = ""
                serialConnecting.text = "Raspberry Pi is connecting to Arduino"
            
            # SDL draw loop: Clear, Draw, Present
            SDL_RenderClear(renderer)
            videoStream = SDL_CreateTextureFromSurface(renderer, windowSurf)
            
            # Draws videoStream onto renderer
            SDL_RenderCopy(renderer, videoStream, None, None)
            
            # Draw received sensor data onto renderer
            if HUDOn:
                for data in dataVals:
                    writeDataToScreen(data, renderer)

            # Draw help if help toggled
            if HUDOn and displayInstructions:
                for dataHelp in dataHelps:
                    writeDataToScreen(dataHelp, renderer)

            # Keep writing these instructions at all times
            writeDataToScreen(toggleHud, renderer)
            writeDataToScreen(serialConnecting, renderer)

            # Present to screen
            SDL_RenderPresent(renderer)

            # Clean up
            SDL_DestroyTexture(videoStream)

        cv2.waitKey(1) # Need this or else program doesn't work

    # Clean up / Quit
    SerialThread.join()
    s.close()
    stream.close()
    SDL_DestroyRenderer(renderer)
    SDL_FreeSurface(windowSurf)
    SDL_DestroyWindow(window)
    cv2.destroyAllWindows()
    TTF_Quit()
    sdl2.ext.quit()
    print("Exiting")



if __name__ == '__main__':
    main()



