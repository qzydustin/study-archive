// Constant value diclaration, object will not be able to change during the process
export const Action=Object.freeze({
    loadMeetings:'loadMeetings',
    loadUser:'loadUser',
    showLoginError:'showLoginError',
    changeView:'changeView',
    userLogout:'userLogout',
    changeOption:'changeOption',
    changePage:'changePage',
    loadCandidate:'loadCandidate',
    loadParticipant:'loadParticipant',
    loadLocation:'loadLocation',
    loadMessage:'loadMessage',
    removeMeeting:'removeMeeting',
    insertMessage:'insertMessage',
    insertFeedback:'insertFeedback',
    changeFeedbackOption:'changeFeedbackOption',
});
// Update the state of loadMeetings
function loadMeetings(meetings){
    return{
        type:Action.loadMeetings,
        payload:meetings,
    };         
}
// Update the state of loadUser
function loadUser(user){
    return{
        type:Action.loadUser,
        payload:user,
    };
}
// Update the state of showLoginError
function showLoginError(){
    return{
        type:Action.showLoginError,
    };
}
// Update the state of loadCandidate
function loadCandidate(candidate){
    return{
        type:Action.loadCandidate,
        payload:candidate,
    };
}
// Update the state of loadParticipant
function loadParticipant(participant){
    return{
        type:Action.loadParticipant,
        payload:participant,
    };
}
// Update the state of loadLocation
function loadLocation(location){
    return{
        type:Action.loadLocation,
        payload:location,
    };
}
// Update the state of loadMessage
function loadMessage(message){
    return{
        type:Action.loadMessage,
        payload:message,
    };
}
// Update the state of removeMeeting
function removeMeeting(id){
    return{
        type:Action.removeMeeting,
        payload:id,
    };
}
// Update the state of insertMeeting
function insertMessage(message){
    return{
        type:Action.insertMessage,
        payload:message,
    };
}
// Update the state of insertFeedback
function insertFeedback(feedback){
    return{
        type:Action.insertFeedback,
        payload:feedback,
    };
}
// Update the state of changeView
export function changeView(id){
    return{
        type:Action.changeView,
        payload:id,
    };
}
// Update the state of userLogout
export function userLogout(){
    return{
        type:Action.userLogout,
    };
}
// Update the state of changeOption
export function changeOption(option){
    return{
        type:Action.changeOption,
        payload:option,
    };
}
// Update the state of changePage
export function changePage(page){
    return{
        type:Action.changePage,
        payload:page,
    };
}
// Update the state of changeFeedbackOption
export function changeFeedbackOption(option){
    return{
        type:Action.changeFeedbackOption,
        payload:option,
    };
}
// Handling user inputs by "dispatching" action objects, runs the reducer, calculates the upload state
// and runs the subscribers to update the UI
// Describe what should happened in the app
const host='http://127.0.0.1:8080';
// respond to the user action and connects to the database for data contains Meetings
export function findAllMeetings(){
    return dispatch=>{
        fetch(`${host}/getAllMeetings`)
        .then(response=>response.json())
        .then(data=>{
            dispatch(loadMeetings(data));
        })
        .catch(error=>console.log(error));
    };
};
// respond to the user action and connects to the database for data contains users
export function findUser(email,password){
    return dispatch=>{
        fetch(`${host}/getUser/${email}/${password}`)
        .then(response=>response.json())
        .then(data=>{
            if(password===data.upassword){
                dispatch(loadUser(data));
                dispatch(findMeetingsByUserId(data.uid));
            }
            else{
                dispatch(showLoginError());
            }
        })
        .catch(error=>console.log(error));
    };
};
// respond to the user action and connects to the database for data contains Meetings by the user id
export function findMeetingsByUserId(id){
    return dispatch=>{
        fetch(`${host}/getMeetingsByUserId/${id}`)
        .then(response=>response.json())
        .then(data=>{
            dispatch(loadMeetings(data));
        })
        .catch(error=>console.log(error));
    };
}
// respond to the user action and connects to the database for creating Meeting
export function createMeeting(userList,startTime,endTime,lid){
    var status=0;
    var feedback=null;
    const meeting={userList,startTime,endTime,lid,status,feedback};
    const options ={
        method: 'POST',
        headers:{
            'Content-Type': 'application/json',
        },
        // Stringify converts from object or value to JSON String
        body:JSON.stringify(meeting),
    }
    fetch(`${host}/insertMeeting`, options)
    .catch(error=>console.log(error));
}
// respond to the user action and connects to the database for deleting Meeting
export function deleteMeeting(mid){
    return dispatch=>{
        const options ={
            method: 'DELETE',
            headers:{
                'Content-Type': 'application/json',
            },
        }
        fetch(`${host}/deleteMeeting/${mid}`, options)
        .then(()=>{
            dispatch(removeMeeting(mid));
        })
        .catch(error=>console.log(error));
    };
}
// respond to the user action and connects to the database for creating Account
export function createAccount(uname,email,upassword,phoneNumber,type){
    const user={uname,email,upassword,phoneNumber,type}
    const options ={
        method:'POST',
        headers:{
            'Content-Type': 'application/json',
        },
        // Stringify converts from object or value to JSON String
        body:JSON.stringify(user),
    }
    fetch(`${host}/insertUser`,options)
    .catch(error=>console.log(error));
}
// respond to the user action and connects to the database for getting Candidate
export function findCandiate(){
    return dispatch=>{
        fetch(`${host}/getCandidate`)
        .then(response=>response.json())
        .then(data=>{
            dispatch(loadCandidate(data));
        })
        .catch(error=>console.log(error));
    };
}
// respond to the user action and connects to the database for getting Participant
export function findParticipant(){
    return dispatch=>{
        fetch(`${host}/getParticipant`)
        .then(response=>response.json())
        .then(data=>{
            dispatch(loadParticipant(data));
        })
        .catch(error=>console.log(error));
    };
}
// respond to the user action and connects to the database for getting location details 
export function findLocation(){
    return dispatch=>{
        fetch(`${host}/getLocation`)
        .then(response=>response.json())
        .then(data=>{
            dispatch(loadLocation(data));
        })
        .catch(error=>console.log(error));
    };
}
// respond to the user action and connects to the database for getting Message details
export function findMessage(id){
    return dispatch=>{
        fetch(`${host}/getMessage/${id}`)
        .then(response=>response.json())
        .then(data=>{
            dispatch(loadMessage(data));
        })
        .catch(error=>console.log(error));
    };
}
// respond to the user action and connects to the database for insert Message details
export function addMessage(b_id,u_name,m_body,postTime){
    return dispatch=>{
        const message={b_id,u_name,m_body,postTime};
        const options ={
            method:'POST',
            headers:{
                'Content-Type': 'application/json',
            },
            // Stringify converts from object or value to JSON String
            body:JSON.stringify(message),
        }
        fetch(`${host}/insertMessage`,options)
        .then(response=>response.json())
        .then(data=>{
            dispatch(insertMessage(data));
        })
        .catch(error=>console.log(error));
    };
}
// respond to the user action and connects to the database for insert Feedback details
export function addFeedback(mid,feedback){
    return dispatch=>{
        const info={mid,feedback};
        const options ={
            method:'PUT',
            headers:{
                'Content-Type': 'application/json',
            },
            // Stringify converts from object or value to JSON String
            body:JSON.stringify(info),
        }
        fetch(`${host}/addFeedback`,options)
        .then(()=>{
            dispatch(insertFeedback(info));
        })
        .catch(error=>console.log(error));
    };
}