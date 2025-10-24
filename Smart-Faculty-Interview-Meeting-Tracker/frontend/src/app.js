// Importation of all used library and functions
import React from 'react';
import './css/app.css';
import {MeetingTracker} from './meeting-tracker.js';
import {SideBar} from './side-bar.js';
import {Login} from './login.js';
import {CreateMeeting} from './create-meeting.js';
import {CreateAccount} from './create-account.js';
import {MyFile} from './my-file.js';
import {MyAccount} from './my-account.js';
import {useDispatch, useSelector} from 'react-redux';
import {userLogout} from './actions.js';
// pages in the app 
const pages = ["", "My Meetings", "My Files", "My Account", "Create Meeting", "Create Account"];
// Main 
function App() {
    const dispatch = useDispatch();
    const meetings = useSelector(state => state.meetings);
    const currentPage = useSelector(state => state.currentPage);
    const username = useSelector(state => state.user.uname);

  return (
    <div id="meeting_tracker_root">
      <div className="Header_Info">
        <p>Welcome! {username}</p>
        {/* Printing and Logout Services */}
        {(currentPage!==0)&&<div onClick={() => {window.print()}}>Print</div>}
        {(currentPage!==0)&&<div onClick={()=>logout()}>Logout</div>}
      </div>
      <SideBar/>
      <div id="main">
        <div id="title">{pages[currentPage]}</div>
        {/* Different pages indicate different tabs */}
        {(currentPage===0)&&<Login/>}
        {(currentPage===1)&&meetings.map(meeting=><MeetingTracker key={meeting.mid} info={meeting}/>)}
        {(currentPage===2)&&<MyFile/>}
        {(currentPage===3)&&<MyAccount/>}
        {(currentPage===4)&&<CreateMeeting/>}
        {(currentPage===5)&&<CreateAccount/>}
      </div>
    </div>
  );
  // Logout function
  function logout(){
    dispatch(userLogout());
  }
}

export default App;