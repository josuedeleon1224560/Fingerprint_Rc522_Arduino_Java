/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 */

package com.mycompany.learning_finger_card;

import com.panamahitek.ArduinoException;
import com.panamahitek.PanamaHitek_Arduino;
import com.panamahitek.PanamaHitek_MultiMessage;
import java.util.logging.Level;
import java.util.logging.Logger;
import jssc.SerialPortEvent;
import jssc.SerialPortEventListener;
import jssc.SerialPortException;
import java.net.URI;
import java.net.URISyntaxException;

/**
 *
 * @author josue
 */



                 

public class Learning_Finger_Card {
    static PanamaHitek_Arduino ino = new PanamaHitek_Arduino();
    static PanamaHitek_MultiMessage multi = new PanamaHitek_MultiMessage(2,ino);
    static SerialPortEventListener listener = new SerialPortEventListener() {
       @Override
        public void serialEvent(SerialPortEvent serialPortEvent) {            
           try {
               if(multi.dataReceptionCompleted())
               {
                   multi.flushBuffer();
                   if("Match".equals(multi.getMessage(0)) || "ID_Tarjeta".equals(multi.getMessage(0)))
                   {
            String baseUrl = "https://postman-echo.com";
            String code = multi.getMessage(1);
            String name ="prueba";

            // Crear una URI con los parámetros de consulta
            URI uri = new URI(baseUrl + "?code=" + code + "&name=" + name);

            System.out.println("URL completa: " + uri.toString());
                   }
                   
               }
           } catch (ArduinoException | SerialPortException | URISyntaxException ex) {
               Logger.getLogger(Learning_Finger_Card.class.getName()).log(Level.SEVERE, null, ex);
           }
        }
    };


        
    
    public static void main(String[] args) throws ArduinoException {
        try
        {
         ino.arduinoRXTX("COM5", 9600,listener);     
        }
         catch (ArduinoException ex) {
Logger.getLogger(Learning_Finger_Card.class.getName()).log(Level.SEVERE, null, ex);
                }
        
}
            }
