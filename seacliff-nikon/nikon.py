import sys
import os
import subprocess
import datetime

class nikonError(Exception):
    def __init__(self, value):
        Exception.__init__(self)
        self.value = value

    def __str__(self):
        return repr(self.value)

class nikon():
    def __init__(self):
        self.name = "nikon"

    def capture(self, filename = "test.png", iso = "500", shutter = "1/30", aperture = "1.8"):
        camera_command = 'CameraControlCmd.exe'
        camera_command_details = '/filename ./' + filename + ' /capture /iso '+ iso + ' /shutter ' + shutter + ' /aperture' + aperture
        print('camera details = ',camera_command_details)
        full_command=camera_command + ' ' + camera_command_details
        p = subprocess.Popen(full_command, stdout=subprocess.PIPE, universal_newlines=True, shell=False)
        (output, err) = p.communicate()

        #This makes the wait possible
#        p_status = p.wait(1)
        # print(p.stdout.readline())

        #This will give you the output of the command being executed
        print('Command output: ' + str(output))
        print('Command err: ' + str(err))

        print('done')


if __name__ == "__main__":
    rawimagename= "test.png"
    print('Name of raw image will be: ', rawimagename)
    the_camera = nikon()
    the_camera.capture(rawimagename)