rm libsai.so
gcc -g -shared -o libsai.so -fPIC -I inc/sai/inc -I inc -Wall sai.c db.c sai_utilities.c
sudo install libsai.so /usr/local/lib
sudo install libsai.so /usr/lib64
ldconfig

