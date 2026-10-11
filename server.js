const express = require('express');
const cors = require('cors');
const app = express();
const PORT = process.env.PORT || 3000;

app.use(cors());
app.use(express.json());

app.get('/', (req, res) => {
  res.status(200).json({ status: 'success', message: 'BCI Server OK' });
});

app.post('/api/logs', (req, res) => {
  console.log('\n--- [ESP32 LOG RECEIVED] ---');
  console.log('Nội dung:', JSON.stringify(req.body, null, 2));
  res.status(200).json({ status: 'success' });
});

app.post('/api/telemetry', (req, res) => {
  console.log('\n=== [ESP32 TELEMETRY] ===');
  console.log('Chỉ số:', JSON.stringify(req.body, null, 2));
  res.status(200).json({ status: 'success' });
});

app.listen(PORT, () => {
  console.log(`Server chạy tại port ${PORT}`);
});