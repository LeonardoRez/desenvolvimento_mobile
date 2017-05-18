import { Component } from '@angular/core';
import {Paho} from 'ng2-mqtt/mqttws31';

@Component({
  selector: 'page-home',
  templateUrl: 'home.html'
})
export class HomePage {
  public temp;//valor da ultima temperatura lida
  public umAr;//valor da ultima umidade do ar lida
  public umSolo;//valor da ultima umidade do solo lida
  client;//objeto que faz comunicação com o broker

  constructor() {
    this.temp = "Não medido";
    this.umAr = "Não medido";
    this.umSolo = "Não medido";

    this.client = new Paho.MQTT.Client('m13.cloudmqtt.com', 37093, 'qwerty12345');
    var options = {
      useSSL: true,
      userName: "cdhwvxkk",
      password: "uoHN9SWskIFf",
      onSuccess: this.onConnected.bind(this)
      //onFailure:doFail
    }
    this.onMessage();
    this.onConnectionLost();
    this.client.connect(options);
  }

  onConnected() {
    console.log("Connected");
    this.client.subscribe("/sistema/#");
    this.client.subscribe("/sensor/#");
  }

  sendMessage(message: string, destination: string) {
    let packet = new Paho.MQTT.Message(message);
    packet.destinationName = destination;
    this.client.send(packet);
  }

  onMessage() {
    this.client.onMessageArrived = (message: Paho.MQTT.Message) => {
      console.log('Message arrived : ' +message.destinationName+"   "+ message.payloadString);
      switch(message.destinationName){
        case "/sensor/temp/refresh":{
          this.temp = message.payloadString+"ºC"
          break;
        }
        case "/sensor/umiar/refresh":{
          this.umAr = message.payloadString+"%"
          break;
        }
        case "/sensor/umisolo/refresh":{
          this.umSolo = message.payloadString
          break;
        }

      }
    };
  }

  onConnectionLost() {
    this.client.onConnectionLost = (responseObject: Object) => {
      console.log('Connection lost : ' + JSON.stringify(responseObject));
    };
  }

  evento() {
        console.log('tentando enviar msg');
        this.sendMessage("enviada com sucesso","/teste");
        console.log('sucesso?');
  }
  public irrigar(){
    this.sendMessage("1","/sistema/irrigar");
  }
  public medirTemp(){
    this.sendMessage("1","/sensor/temp/status");
  }
  public medirUmidAr(){
    this.sendMessage("1","/sensor/umiar/status");
  }
  public medirUmidTerra(){
    this.sendMessage("1","/sensor/umisolo/status");
  }
}
