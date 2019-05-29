rm libsai.so
gcc -shared -o libsai.so -fPIC -I inc/sai/inc -I inc -Wall sai.c db.c sai_utilities.c
sudo install libsai.so /usr/local/lib
ldconfig

