// Importation of all used library and functions
import React from 'react';
import ReactDOM from 'react-dom';
import './css/index.css';
import App from './app';
import {Provider} from 'react-redux';
import store from './store.js';
// Render a React element into the DOM in the the supplies of store and return a reference to the component
// This will perform an update on it and only mutate the DOM as necessary to reflect the latest React Element
ReactDOM.render(
  <React.StrictMode>
    <Provider store={store}>
      <App />
    </Provider>
  </React.StrictMode>,
  document.getElementById('root')
);

 



