function checkEmail(strEmail) {
    var reg = /^\w+([-+.]\w+)*@uwec.edu/;
    console.log(strEmail);
    var li = document.getElementById("result");
    if (strEmail != null && strEmail.search(reg) != -1) {
        li.style.color = "red";
        li.innerHTML = "<font color='green' size='4px'>Success</font>";
    } else {
        li.style.color = "green";
        li.innerHTML = "<font color='red' size='4px'>It is not an UWEC email</font>";
    }
}