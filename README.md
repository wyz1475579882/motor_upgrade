
### To install dependencies:
    sudo git clone http://git.ddt.dev:9281/wuyunzhou/motor_upgrade.git
    sudo cp -r motor_upgrade/ /usr/
    sudo cp /usr/motor_upgrade/ota_lib/*.so /usr/lib
    sudo cp /usr/motor_upgrade/ota_lib/otafifth_demo /usr/bin
    sudo pip install pycryptodome
    sudo pip install crcmod
    sudo chmod 777 /usr/bin/otafifth_demo
    
### To run control board upgrade:
    otafifth_demo -f $BIN_PATH
    
### To run motor upgrade:
    sudo /usr/motor_upgrade/run.sh -t $MOTOR_TYPE -i $MOTOR_ID -s $CANBUS_ID -b $MOTOR_BIN_PATH

### To get motor lastest bin:
    git clone http://git.ddt.dev:9281/wuyunzhou/motor-patch.git

### To auto upgrade all motor and control board:
    cd motor-patch/
    sudo chmod 777 run.sh
    sudo ./run.sh

### To make motor_upgrade_deb.deb:
    chmod 0755 motor_upgrade_deb/DEBIAN/postinst
    fakeroot dpkg-deb -b motor_upgrade_deb

### To install motor_upgrade_deb.deb:
    sudo dpkg -i motor_upgrade_deb.deb