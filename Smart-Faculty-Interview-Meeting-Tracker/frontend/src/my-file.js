// Importation of all used library and functions
import React from 'react';
import './css/my-file.css';
import Upload from './upload';

// Page to Upload a File
// Currently in use without function
export function MyFile() {
    return (
        <div>
            <div><Upload/></div>
            {/*<div className="tag-small5">Current File stored</div>*/}
            {/*<div className="info-small2">Myresume.txt</div>*/}
        </div>
    );
}