// Importation of all used library and functions
import {createStore,applyMiddleware} from 'redux';
import thunk from 'redux-thunk';
import reducer from './reducer.js';
// call action creators and return a function instead of an action object
// function receives the store's dispatch method, then used to dispatch synchronous actions
// inside the function's body once the asychronous operations have been completed
const store=createStore(reducer,applyMiddleware(thunk));

export default store;