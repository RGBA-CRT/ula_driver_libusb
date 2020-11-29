
gcc -c ezusb.c -o ezusb.o -O0 -g -I..\include\libusb-1.0 -L..\MinGW32\static -lusb-1.0 
gcc -c ula_driver.c -o ula_driver.o -O0 -g -I..\include\libusb-1.0 -L..\MinGW32\static -lusb-1.0 
gcc -c ula_test.c -o ula_test.o -O0 -g -I..\include\libusb-1.0 -L..\MinGW32\static -lusb-1.0 
gcc -c dllmain.c -o dllmain.o -O0 -g -I..\include\libusb-1.0 -L..\MinGW32\static -lusb-1.0 
gcc -c ula_gp.c -o ula_gp.o -O0 -g -I..\include\libusb-1.0 -L..\MinGW32\static -lusb-1.0 
gcc ula_driver.o ezusb.o ula_test.o -o ula_driver.exe -L..\MinGW32\static -lusb-1.0 
gcc ula_driver.o ezusb.o ula_gp.o -o ula_gp.exe -L..\MinGW32\static -lusb-1.0 
gcc ula_driver.o ezusb.o dllmain.o ula_driver.def -O0 -g  --disable-stdcall-fixup -shared -o ula_driver.dll -Wl,--out-implib,ula_driver.lib -L..\MinGW32\dll -lusb-1.0 
strip ula_driver.dll