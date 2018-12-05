void initWebServer(){
  //Initializing server
  server.on("/list", handleListFiles);
  server.on("/edit", HTTP_GET, []{
    if(!handleReadFile("/edit.html"))
      server.send(404, "text/plain", "404; Not Found");
  });
  server.on("/edit", HTTP_POST, []{
      server.send(200);
      }, handleUploadFile);
  server.on("/edit", HTTP_DELETE,
      handleDeleteFile);
  server.on("/", []{
    if(!handleReadFile("/index.html"))
      server.send(404, "text/plain", "404; Not Found");
  });
  server.onNotFound([](){
      if(!handleReadFile(server.uri())){
        server.send(404,"text/plain", "FileNotFound");
      }  
    });
  server.begin();
}

String getFileFormat(String fileName){
  if(fileName.endsWith(".html"))    return  "text/html";
  else if(fileName.endsWith(".jpg")) return  "image/jpg";
  else                              return  "text/plain";
}
bool handleReadFile(String path){
  Serial.println("File read: "+path);
  if(SPIFFS.exists(path)){
    String fileFormat = getFileFormat(path);
    File file = SPIFFS.open(path, "r");
    server.streamFile(file, fileFormat);
    file.close();
    return true;
  }
  return false;
}

void handleUploadFile(){
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String fileName = upload.filename;
    if(!fileName.startsWith("/"))
      fileName = "/"+fileName;
    Serial.println("File upload: "+fileName);
    fsUploadFile = SPIFFS.open(fileName, "w");
    fileName = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile){
      fsUploadFile.close();
      Serial.print("Succesful Upload: ");
      Serial.println(upload.totalSize);
      server.sendHeader("Location","/success.html");
      server.send(303);
    } 
  } else{
    server.send(500, "text/plain", "500: couldn't create file");
  }
}

void handleDeleteFile(){
  if(server.args() == 0){
    server.send(422, "text/plain", "BAD ARGS");
    return;
  }
  String path = server.arg(0);
  Serial.println("File delete: "+path);
  if(path == "/"){
    server.send(422, "text/plain", "EMPTY PATH");
    return;
  }
  if(!SPIFFS.exists(path)){
    server.send(404, "text/plain", "FileNotFound");
    return;
  }
  SPIFFS.remove(path);
  server.send(200, "text/plain", "File deleted");
}

void handleListFiles(){
  String temp = "File list"+'\n';
  Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      temp = temp + dir.fileName() + '\n';
    }
  server.send(200, "text/plain", temp);
}

