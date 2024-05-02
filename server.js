const express = require('express');
const fs = require('fs');
const http = require('http');
const socketIo = require('socket.io');

const app = express();
const server = http.createServer(app);
const io = socketIo(server);


app.use(express.static('public'));
app.use(express.json());

app.get('/api/cloche', (req, res) => {
  fs.readFile('conf/horaires.json', 'utf8', (err, data) => {
    if (err) {
      res.status(500).json({ error: 'Erreur lors de la lecture du fichier de configuration.' });
      return;
    }
    res.json(JSON.parse(data));
  });
});

app.post('/api/cloche', (req, res) => {
  const horaires = req.body.horaires;
  fs.writeFile('conf/horaires.json', JSON.stringify(horaires), err => {
    if (err) {
      console.error('Erreur lors de l\'écriture du fichier d\'horaires :', err);
      res.status(500).json({ error: 'Erreur lors de l\'enregistrement des modifications d\'horaires.' });
      return;
    }
    io.emit('horairesModifies', horaires);
    res.json({ success: true });
  });
});

server.listen(8080, '192.168.0.119', () => {
  console.log(`Serveur lancé sur http://192.168.0.119:8080`);
});