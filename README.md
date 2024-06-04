# Ring The Bell

This project aims to automate the ringing of a bell based on predefined schedules fetched from an API. When the current time matches a predefined time slot, the bell will ring accordingly. The project consists of both Arduino code to control the bell and a Node.js Express server along with an API to provide the schedule data.

![Node.js Logo](https://upload.wikimedia.org/wikipedia/commons/thumb/d/d9/Node.js_logo.svg/110px-Node.js_logo.svg.png) ![Arduino Logo](https://upload.wikimedia.org/wikipedia/commons/thumb/8/87/Arduino_Logo.svg/110px-Arduino_Logo.svg.png)

## Getting Started

To get started with this project, you'll need the following components:

- Arduino board compatible with WiFi (e.g., ESP8266 or ESP32)
- Bell mechanism connected to the Arduino
- WiFi network to connect the Arduino to
- Node.js installed on your development machine to run the server

## Setting Up Arduino

1. Connect your Arduino board to your computer via USB.
2. Upload the provided Arduino code (`Bell_Ring_Script.ino`) to your Arduino board using the Arduino IDE or any compatible IDE.
3. Make sure to update the following variables in the Arduino code with your WiFi credentials and server information:
   - `ssid`: Your WiFi network name
   - `password`: Your WiFi password
   - `host`: IP address of your server
   - `port`: Port number used by your server

## Setting Up Node.js Server

1. Navigate to the `server` directory in this repository.
2. Install the necessary dependencies by running `npm install`.
3. Start the server by running `npm start`.
4. By default, the server runs on port 8085. Make sure this port is accessible and not blocked by your firewall.

## Usage

1. Once both the Arduino and the server are set up, the Arduino will automatically connect to your WiFi network and start fetching schedule data from the server.
2. The server provides schedule data via a RESTful API. Make sure the API is accessible from the Arduino's network.
3. Schedule data should be provided in JSON format with the following structure:
   ```json
   {
     "lundi": "[Schedule data for Monday]",
     "mardi": "[Schedule data for Tuesday]",
     ...
   }
4. The Arduino will compare the current time with the schedule data fetched from the server and ring the bell accordingly.

## Contributing

Contributions are welcome! If you have any improvements or feature suggestions, feel free to open an issue or submit a pull request.
