import React, { useState } from "react";
import { useDispatch } from "react-redux";
import {
  edit,
  updateOrder,
  deleteOrder,
  loadItems,
  loadUrgent,
} from "./actions";
export function Order(props) {
  const order = props.order;
  const dispatch = useDispatch();

  const [items, setItems] = useState(order.items);
  const [amount, setAmount] = useState(order.amount);
  const [note, setNote] = useState(order.note);
  const [urgent, setUrgent] = useState(order.urgent);

  const onEdit = () => {
    dispatch(edit(order));
  };

  const onSave = () => {
    dispatch(
      updateOrder({
        id: order.id,
        items,
        amount,
        note,
        urgent,
      })
    );
  };

  const onDelete = () => {
    dispatch(deleteOrder(order));
  };

  const onSearchItems = () => {
    dispatch(loadItems(order.items));
  };

  const onSearchUrgent = () => {
    dispatch(loadUrgent(order.urgent));
  };

  if (order.isEditing) {
    return (
      <div className="order">
        <div className="edit-items">
          <input
            type="text"
            value={items}
            onChange={(e) => setItems(e.target.value)}
          />
        </div>
        <div className="edit-amount">
          <input
            type="text"
            value={amount}
            onChange={(e) => setAmount(e.target.value)}
          />
        </div>
        <div className="edit-note">
          <input
            type="text"
            value={note}
            onChange={(e) => setNote(e.target.value)}
          />
        </div>
        <div className="edit-urgent">
          <input
            type="text"
            value={urgent}
            onChange={(e) => setUrgent(e.target.value)}
          />
        </div>
        <div className="save">
          <button onClick={onSave}>save</button>
        </div>
      </div>
    );
  } else {
    return (
      <div className="order">
        <div className="items" onClick={onSearchItems}>
          {order.items}
        </div>
        <div className="amount">{order.amount}</div>
        <div className="note">{order.note}</div>
        <div className="urgent" onClick={onSearchUrgent}>
          {order.urgent}
        </div>
        <div className="edit" onClick={onEdit}>
          <button>Edit</button>
        </div>
        <div className="delete-button" onClick={onDelete}>
          &#x2716;
        </div>
      </div>
    );
  }
}
