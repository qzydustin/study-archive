import { Action } from "./actions";
const initialState = {
  isWaiting: false,

  orders: [],
};

function reducer(state = initialState, action) {
  switch (action.type) {
    case Action.LoadOrder:
      return {
        ...state,
        orders: action.payload,
      };
    case Action.AfterAdd:
      return {
        ...state,
        orders: [{ ...action.payload, isEditing: true }, ...state.orders],
      };
    case Action.EditOrder:
      return {
        ...state,
        orders: state.orders.map((order) => {
          if (order.id === action.payload.id) {
            return { ...order, isEditing: true };
          } else {
            return order;
          }
        }),
      };
    case Action.AfterEdit:
      return {
        ...state,
        orders: state.orders.map((order) => {
          if (order.id === action.payload.id) {
            return { ...order, isEditing: undefined };
          } else {
            return order;
          }
        }),
      };
    case Action.AfterSave:
      return {
        ...state,
        orders: state.orders.map((order) => {
          if (order.id === action.payload.id) {
            return action.payload;
          } else {
            return order;
          }
        }),
      };
    case Action.AfterDelete:
      return {
        ...state,
        orders: state.orders.filter((order) => order.id !== action.payload.id),
      };
    default:
      return state;
  }
}

export default reducer;
