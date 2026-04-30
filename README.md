# Dual-Microcontroller-Communication-and-Data-Pipeline-System
This project is to learn how to create a fault tolerant embedded data acquisition system using two microcontrollers with checksum validation, retry logic, and real-time Python based data processing and visualization

## Master Microcontroller
This microcontroller is in charge of sending and recieving data from the slave. It contains the function `i2c_master_write()` where it writes a byte to the slave microntroller in which determines the sensor that the slave will read and send back to the master. It also contains the function `i2c_master_recieve()` which recieves the requested data from the slave microcontroller. Future implementation will allow for the master to send data to a computer for python data processing.

## Slave microcontroller
This microcontroller reads data from sensors, and listens to instruction from the master. This microcontroller will also have validation and retry logic for error control.