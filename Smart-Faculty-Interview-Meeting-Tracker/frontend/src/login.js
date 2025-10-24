// Importation of all used library and functions
import React from 'react';
import './css/login.css';
import {useDispatch, useSelector} from 'react-redux';
import logo from './images/logo.png';
import {findUser} from './actions.js';
// Funtion for checking user logins 
export function Login(){
    const dispatch=useDispatch();
    const loginError=useSelector(state=>state.loginError);
    return(
        <div id="login">
            <div id="login-header">
                <img src={logo} alt="Logo"/>
            </div>
            <p>E-mail:</p>
            <input id="email" type="text" placeholder="Enter your email"/>
            <p>Password:</p>
            <input id="password" type="password" placeholder="Enter your password"/>
            <div id="login-error">{loginError&&error()}</div>
            <div id="login-button" onClick={login}>Login</div>
        </div>
    );
    // prompt for error message
    function error(){
        return "Invaild e-mail or password!";
    }
    // Getting user login credentials and check on our database
    function login(){
        dispatch(findUser(document.getElementById('email').value,document.getElementById('password').value));
    }
}