const host = "http://localhost:81";

export const Action = Object.freeze({
  LoadOrder: "LoadOrder",
  AfterAdd: "AfterAdd",
  EditOrder: "EditOrder",
  AfterEdit: "AfterEdit",
  AfterSave: "AfterSave",
  AfterDelete: "AfterDelete",
});

export class Items {
  constructor(items) {
    this.items = items;
  }
}

export class Amount {
  constructor(amount) {
    this.amount = amount;
  }
}

export class Note {
  constructor(note) {
    this.note = note;
  }
}

export class Urgent {
  constructor(urgent) {
    this.urgent = urgent;
  }
}

export class ONE {
  constructor(items, amount, note, urgent) {
    this.items = items;
    this.amount = amount;
    this.note = note;
    this.urgent = urgent;
  }
}

function check(response) {
  if (!response.ok) {
    throw Error(`${response.status}: ${response.statusText}`);
  }
  return response;
}

export function loadOrder(order) {
  return {
    type: Action.LoadOrder,
    payload: order,
  };
}

export function afterAdd(order) {
  return {
    type: Action.AfterAdd,
    payload: order,
  };
}

export function afterSave(order) {
  return {
    type: Action.AfterSave,
    payload: order,
  };
}

export function afterDelete(order) {
  return {
    type: Action.AfterDelete,
    payload: order,
  };
}

export function edit(order) {
  return {
    type: Action.EditOrder,
    payload: order,
  };
}

export function leaveEditMode(order) {
  return {
    type: Action.AfterEdit,
    payload: order,
  };
}

export function loadAllOrders() {
  return (dispatch) => {
    fetch(`${host}/allorders`)
      .then(check)
      .then((response) => response.json())
      .then((data) => {
        if (data.ok) {
          dispatch(loadOrder(data.orders));
        }
      })
      .catch((e) => console.error(e));
  };
}

export function loadUrgent() {
  return (dispatch) => {
    fetch(`${host}/urgent`)
      .then(check)
      .then((response) => response.json())
      .then((data) => {
        if (data.ok) {
          dispatch(loadOrder(data.orders));
        }
      })
      .catch((e) => console.error(e));
  };
}

export function loadItems(items) {
  return (dispatch) => {
    fetch(`${host}/items/${items}`)
      .then(check)
      .then((response) => response.json())
      .then((data) => {
        if (data.ok) {
          dispatch(loadOrder(data.orders));
        }
      })
      .catch((e) => console.error(e));
  };
}

export function addOrder(items, amount, note, urgent) {
  const order = { items, amount, note, urgent };
  const options = {
    method: "POST",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(order),
  };
  return (dispatch) => {
    fetch(`${host}/order`, options)
      .then(check)
      .then((response) => response.json())
      .then((data) => {
        if (data.ok) {
          order.id = data.id;
          dispatch(afterAdd(order));
        }
        console.log(data);
      })
      .catch((e) => console.error(e));
  };
}

export function updateOrder(order) {
  const options = {
    method: "PATCH",
    headers: {
      "Content-Type": "application/json",
    },
    body: JSON.stringify(order),
  };
  return (dispatch) => {
    fetch(`${host}/order/${order.id}`, options)
      .then(check)
      .then((response) => response.json())
      .then((data) => {
        if (data.ok) {
          dispatch(afterSave(order));
        }
      })
      .catch((e) => console.error(e));
  };
}

export function deleteOrder(order) {
  const options = {
    method: "DELETE",
  };

  return (dispatch) => {
    fetch(`${host}/order/${order.id}`, options)
      .then(check)
      .then((response) => response.json())
      .then((data) => {
        if (data.ok) {
          dispatch(afterDelete(order));
        }
      })
      .catch((e) => console.error(e));
  };
}
