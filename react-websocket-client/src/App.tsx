import React, { useState, useEffect, useRef } from 'react';
import { w3cwebsocket as W3CWebSocket } from 'websocket';
import './App.css';

const App: React.FC = () => {
  const [selectedFile, setSelectedFile] = useState<File>();
  const [client, setClient] = useState<W3CWebSocket>();
  const fileInputRef = useRef<HTMLInputElement>(null);
  const submitInputRef = useRef<HTMLInputElement>(null);

  useEffect(() => {
    const client = new W3CWebSocket('ws://your-esp32-server-address');
    setClient(client);
  }, []);

  const handleFileChange = (event: React.ChangeEvent<HTMLInputElement>) => {
    setSelectedFile(event.target.files ? event.target.files[0] : undefined);
  };

  const handleFormSubmit = (event: React.FormEvent) => {
    event.preventDefault();
    if (client && selectedFile) {
      const reader = new FileReader();
      reader.onload = function(event) {
        const fileData = event.target?.result;
        if (typeof fileData === 'string') {
          client.send(fileData);
        }
      };
      reader.readAsText(selectedFile);
    }
  };

  return (
    <div className="app">
      <h1>Upload Settings</h1>
      <p>
        <b>Instructions:</b> After selecting your stops and downloading your data.json from <a href='https://oasa.ppdms.gr'>oasa.ppdms.gr</a>, please select it on the drop-down menu here and press send!
      </p>
      <form className="form" onSubmit={handleFormSubmit}>
        <input id="file" ref={fileInputRef} className="input" type="file" accept=".json" onChange={handleFileChange} />
        
        <input id="submit" ref={submitInputRef} className="submit" type="submit" />
        
        <div className="buttons">
        <label htmlFor="file" className="button">Choose file</label>
          <label htmlFor="submit" className="button" onClick={() => submitInputRef.current?.click()}>Send</label>
        </div>
      </form>
    </div>
  );
};

export default App;
