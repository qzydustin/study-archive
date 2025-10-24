const express = require("express");
const cors = require("cors");
const mysql = require("mysql");
const fs = require("fs");

const app = express();
app.use(cors());
app.use(express.json());

let credentials = JSON.parse(fs.readFileSync("credentials.json", "utf8"));
let connection = mysql.createConnection(credentials);
connection.connect();

function rowToObject(row) {
  return {
    id: row.id,
    items: row.items,
    amount: row.amount,
    note: row.note,
    urgent: row.urgent,
  };
}

app.get("/allorders", (request, response) => {
  const query =
    "SELECT items, amount, note, urgent, id  FROM orders WHERE is_deleted = 0";
  connection.query(query, null, (errors, rows) => {
    response.send({
      ok: true,
      orders: rows.map(rowToObject),
    });
  });
});

app.get("/items/:items", (request, response) => {
  const query =
    "SELECT items, amount, note, urgent, id FROM orders WHERE is_deleted = 0 AND items = ?";
  const params = [request.params.items];
  connection.query(query, params, (errors, rows) => {
    response.send({
      ok: true,
      orders: rows.map(rowToObject),
    });
  });
});

app.get("/urgent", (request, response) => {
  const query =
    "SELECT items, amount, note, urgent, id FROM orders WHERE is_deleted = 0 AND urgent = 'Yes'";
  connection.query(query, (errors, rows) => {
    response.send({
      ok: true,
      orders: rows.map(rowToObject),
    });
  });
});

app.post("/order", (request, response) => {
  const query =
    "INSERT INTO orders (items, amount, note, urgent) VALUES (?, ?, ?, ?)";
  const params = [
    request.body.items,
    request.body.amount,
    request.body.note,
    request.body.urgent,
  ];
  connection.query(query, params, (errors, result) => {
    response.send({
      ok: true,
      id: result.insertId,
    });
  });
});

app.patch("/order/:id", (request, response) => {
  const query =
    "UPDATE orders SET items = ?, amount = ?, note = ?, urgent = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?";
  const params = [
    request.body.items,
    request.body.amount,
    request.body.note,
    request.body.urgent,
    request.params.id,
  ];
  connection.query(query, params, (errors, result) => {
    response.send({
      ok: true,
    });
  });
});

app.delete("/order/:id", (request, response) => {
  const query =
    "UPDATE orders SET is_deleted = 1, updated_at = CURRENT_TIMESTAMP WHERE id = ?";
  const params = [request.params.id];
  connection.query(query, params, (errors, result) => {
    response.send({
      ok: true,
    });
  });
});

const port = 81;
app.listen(port, () => {
  console.log(`Backend are live on port ${port}`);
});
