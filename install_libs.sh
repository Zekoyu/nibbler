# SFML (https://www.sfml-dev.org/tutorials/2.5/start-linux.php)
sudo apt-get install libsfml-dev

# For portaudio (http://portaudio.com/docs/v19-doxydocs/compile_linux.html)
sudo apt-get install libasound-dev
cd ./portaudio/lib && ./configure && make && cd ../
ln -s ./portaudio/lib/.libs/libportaudio.so.2 ./portaudio/libportaudio.so