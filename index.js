const express = require('express');
const bodyParser = require('body-parser');
const SerialPort = require('serialport');
const app = express();
const port = 80;
const serialport = new SerialPort('/dev/ttyACM0', { baudRate: 9600 });

app.use(bodyParser.json());

app.post('/Arduino', (req, res, next) => {
    const command = req.body.command;
    serialport.write('s\n', () => {
        // hack it together because fuck the police
        setTimeout(() => {
            serialport.write(`${command}\n`, (err) => {
                setTimeout(() => {
                    serialport.write('g\n', () => {
                        if (err) {
                            return console.log('Error on write: ', err.message);
                        }
                        res.sendStatus(200);
                    });
                }, 200);
            });
        }, 200);
    });
});

app.use('/', express.static('public'));


app.listen(port, () => {
    console.log(`Server listening on port ${port}`);
});