// Importation of all used library and functions
import React from 'react';
import './css/meeting-view.css';
import close from './images/close.png';
import {useDispatch, useSelector} from 'react-redux';
import {changeOption, changeView, findMessage,deleteMeeting,addMessage,changeFeedbackOption,addFeedback} from './actions';
import {SingleUser} from './single-user.js';
import {Message} from './message.js'
// Function for the details of the meetings
export function MeetingView(props) {
    var info = props.info;
    const dispatch = useDispatch();
    // Constant value declaration
    const option = useSelector(state => state.currentOption);
    const messages = useSelector(state => state.message);
    const type = useSelector(state => state.user.type);
    const username = useSelector(state => state.user.uname);
    const feedbackOption = useSelector(state => state.feedbackOption);
    // Initial state
    var message='';
    var feedback='';

    return (
        <div className="overlay">
            <div className="button-bar">
                {/* Side bar options */}
                <div className="buttons" onClick={() => optionChange(1)}>Feedback</div>
                <div className="buttons" onClick={() => {optionChange(2);getMessage(info.mid)}}>Messages</div>
                <div className="buttons" onClick={() => optionChange(3)}>People</div>
                <div className="buttons" onClick={() => {window.print()}}>Print</div>
                {type<4&&<div className="buttons" onClick={() => {deleteThis(info.mid)}}>Delete</div>}
            </div>
            <div className="view">
                <div className="close">
                    <img src={close} alt="close" onClick={()=>closeView()}/>
                </div>
                {/* Details of the Meetings */}
                <div className="tag-small">Candidate</div>
                <div className="tag-small">Participant</div>
                <div className="info-small">{info.userList.filter(user=>user.type===5).map(user=>user.uname+'\n')}</div>
                <div className="info-small">{info.userList.filter(user=>user.type!==5).map(user=>user.uname+'\n')}</div>
                <div className="tag-small">Starting Time</div>
                <div className="tag-small">Ending Time</div>
                <div className="info-small">{parseTime(info.startTime)}</div>
                <div className="info-small">{parseTime(info.endTime)}</div>
                <div className="tag-big">Location</div>
                <div className="info-big">{info.city+'\n'+info.address}</div>
                <div className="tag-big">Status</div>
                <div className="info-big">{mapStatus(info.status)}</div>
            </div>
            {/* Prompt on the right hand for Feedback, Chat and people functions */}
            {option!==0&&<div className="option">
                {option===1&&<div className="feedback">
                    <div className="feedback_content">
                        <h3>Feedback</h3>
                        <textarea readOnly className="feedback_text" onChange={event=>feedback=event.target.value} defaultValue={info.feedback}></textarea>
                    </div>
                    {type<3&&feedbackOption===0&&<div className="option-button" onClick={()=>edit(0)}>Edit</div>}
                    {type<3&&feedbackOption===1&&<div className="option-button" onClick={()=>edit(1)}>Submit</div>}
                </div>}
                {option===2&&<div className="chat">
                    <div className="chat-message">
                        <h3>Message</h3>
                        <div className="chat-body">
                            {messages.map(message=><Message key={message.m_id} message={message}/>)}
                        </div>
                    </div>
                    <textarea className="chat-input" onChange={event=>message=event.target.value}></textarea>
                    <div className="option-button" onClick={()=>addNewMessage(message)}>Send</div>
                </div>}
                {option===3&&<div className="people">
                    <h3>People</h3>
                    <div className="user-list">
                        {info.userList.map(user=><SingleUser key={user.uid} user={user}/>)}
                    </div>
                </div>}
            </div>}
        </div>
    );
    // Showing the details on the time variables 
    function parseTime(time){
        var result=time.substring(5,7)+'.'+time.substring(8,10)+'.'+time.substring(0,4)+'\n'+time.substring(11,19);
        return result;
    }
    // Check for the status for the meeting
    function mapStatus(status){
        if(status===0){
            return 'Open';
        }
        else if(status===1){
            return 'End';
        }
    }
    // Close the prompt of the deails for meeting
    function closeView(){
        dispatch(changeView(0));
    }
    // Dispath the user option on the left hand bar
    function optionChange(option){
        dispatch(changeOption(option));
    }
    // Get the message from the user
    function getMessage(id){
        dispatch(findMessage(id));
    }
    // Delete the Meeting
    function deleteThis(id){
        dispatch(deleteMeeting(id));
        alert('Delete success');
    }
    // Adding a new message 
    function addNewMessage(message){
        if(message!==''){
            var time=new Date();
            time.setMinutes(time.getMinutes() - time.getTimezoneOffset());
            time=time.toJSON().substr(0, 19).replace(/T/,' ');
            dispatch(addMessage(info.mid,username,message,time));
            var x = document.getElementsByClassName("chat-input");
            var i;
            for (i = 0; i < x.length; i++) {
                x[i].value = "";
            }
        }
    }
    // Edditing the Message
    function startEdit(){
        var x = document.getElementsByClassName("feedback_text");
        var i;
        for (i = 0; i < x.length; i++) {
            x[i].readOnly = false;
            x[i].focus();
        }
    }
    // Done with Editing
    function finishEdit(){
        var x = document.getElementsByClassName("feedback_text");
        var i;
        for (i = 0; i < x.length; i++) {
            x[i].readOnly = true;
            x[i].style.cursor="defult";
        }
    }
    // Edit the feedback
    function edit(option){
        if(option===0){
            startEdit();
            dispatch(changeFeedbackOption(1));
        }
        else{
            finishEdit();
            dispatch(changeFeedbackOption(0));
            dispatch(addFeedback(info.mid,feedback));
        }
    }
}