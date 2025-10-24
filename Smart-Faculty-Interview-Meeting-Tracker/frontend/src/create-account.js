// Importation of all used library and functions
import React from 'react';
import './css/create-account.css';
import {createAccount} from './actions.js';

export function CreateAccount(){
    // Initial state 
    var userName='';
    var emailAddress='';
    var password='';
    var userType=-1;
    var phone='';

    return(
        <form>
            {/* Ask for User input for creating an account */}
            <div id = "CreateAccount">
                <p>User Name:</p>
                <div className="input"> 
                    <input type = "text" onChange={event=>setUserName(event)} placeholder = "Name" name = "name"></input>
                </div>
                <p>Email Address:</p>
                <div className="input">
                    <input type="text" onChange={event=>setUEmailAddress(event)} placeholder = "Email" name = "to_email"></input>
                </div>
                <p>Password:</p>
                <div className="input">
                    <input type="text" onChange={event=>setPassword(event)} placeholder = "Password"></input>
                </div>
                <p>Phone Number:</p>
                <div className="input">
                    <input type="text" onChange={event=>setPhone(event)} placeholder = "Phone Number"></input>
                </div>
                <p>User Type:</p>
                <div className="input">
                    <select onChange = {event=>setUserType(event)}>
                        <option value = {-1}></option>
                        <option value={0}>Super Administrator</option>
                        <option value={1}>Department Administrator</option>
                        <option value={2}>Administrator</option>
                        <option value={3}>Meeting Creator</option>
                        <option value={4}>Participant</option>
                        <option value={5}>Candidate</option>
                    </select>
                </div>
                <div type ="submit" id="submit-button" onClick = {()=>submitMeeting()} value="send Message">Submit</div>
            </div>
        </form>
    );
    // Send an event action on User Name
    function setUserName(event){
        userName=event.target.value;
    }
    // Send an event action on User Email Address
    function setUEmailAddress(event){
        emailAddress=event.target.value;
    }
    // Send an event action on User Password
    function setPassword(event){
        password=event.target.value;
    }
    // Send an event action on User Phone
    function setPhone(event){
        phone=event.target.value;
    }
    // Send an event action on User Type
    function setUserType(event){
        userType=event.target.value;
    }
    // Validation Checks on user inputs
    function submitMeeting(){
        if(userName===''){
            alert('Please enter a user name');
        } else if(emailAddress===''){
            alert('Please enter an e-mail');
        }else if(password===''){
            alert('Please enter a passwaord');
        }else if(phone===''){
            alert('Please enter a phone number');
        } else if(userType<0){
            alert('Please choose a user type');
        } else {
            createAccount(userName,emailAddress,password,phone,parseInt(userType));
            alert('Create Account Success!');
        }
    }
}

