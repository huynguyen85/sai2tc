gcc -shared -o libsai.so -fPIC -I SAI/inc -Wall -Wextra  sai.c
sudo install libsai.so /usr/local/lib

