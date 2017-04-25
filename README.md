## Capture packets as pcap

+ How to use

  + Get source code

    ```sh
    git clone https://github.com/Durant35/Velodyne_pcap.git -b cmake
    ```

  + Setup and build

    ```sh
    ./run.sh setup
    ```

  + Use it for capturing

    ```sh
    ./run.sh capture
    ```

  + Clean generated files

    ```sh
    ./run.sh clean
    ```

+ Dependencies

  + 32-bit standard libraries

    ```sh
    sudo dpkg --add-architecture i386
    sudo apt-get update
    sudo apt-get install g++-multilib
    sudo apt-get install lib32stdc++6
    ```

  + libpcap

    ```sh
    sudo apt-get install libpcap0.8-dev:i386
    ```

    ​

