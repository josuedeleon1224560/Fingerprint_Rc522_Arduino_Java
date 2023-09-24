#include <SoftwareSerial.h>
#include <FPM.h>

#define RELAY_PIN 7
/* Enroll fingerprints */

/*  pin #2 is IN from sensor (GREEN wire)
 *  pin #3 is OUT from arduino  (WHITE/YELLOW wire)
 */
SoftwareSerial fserial(2, 3);
int numero;
FPM finger(&fserial);
FPM_System_Params params;

void setup()
{
    digitalWrite(RELAY_PIN, LOW);
    Serial.begin(9600);
    Serial.println("ENROLL test");
    fserial.begin(9600);

    if (finger.begin()) {
        finger.readParams(&params);
        Serial.println("Found fingerprint sensor!");
    } else {
        Serial.println("Did not find fingerprint sensor :(");
        while (1) yield();
    }

    pinMode(RELAY_PIN,OUTPUT);
}

void loop()
{
digitalWrite(RELAY_PIN, HIGH);  
if(Serial.available()>0)
{  
char mensaje = Serial.read();
if(mensaje=='s')
{
  Serial.println("Searching for a free slot to store the template...");
    int16_t fid;
    if (get_free_id(&fid))
        enroll_finger(fid);
    else
        Serial.println("No free slot in flash library!");
}if(mensaje=='d')
{
    Serial.println("Enter the finger ID # you want to delete...");
    int fid = readnumber();
if (fid == 0) {// ID #0 not allowed, try again!
     return;
  }
    Serial.print("Deleting ID #");
    Serial.println(fid);

    deleteFingerprint(fid);
}
  }else
  {
    search_database();
    if(Serial.available()>0)
    {
      char respuesta = Serial.read();
      if(respuesta=='t')
      {
        digitalWrite(RELAY_PIN, LOW);
        delay(9000);
      }else
      {
        digitalWrite(RELAY_PIN,HIGH);
        delay(4000);
      }
    }
  }
}
  
uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

bool get_free_id(int16_t * fid) {
    int16_t p = -1;
    for (int page = 0; page < (params.capacity / FPM_TEMPLATES_PER_PAGE) + 1; page++) {
        p = finger.getFreeIndex(page, fid);
        switch (p) {
            case FPM_OK:
                if (*fid != FPM_NOFREEINDEX) {
                    Serial.print("Free slot at ID ");
                    Serial.println(*fid);
                    return true;
                }
                break;
            case FPM_PACKETRECIEVEERR:
                Serial.println("Communication error!");
                return false;
            case FPM_TIMEOUT:
                Serial.println("Timeout!");
                return false;
            case FPM_READ_ERROR:
                Serial.println("Got wrong PID or length!");
                return false;
            default:
                Serial.println("Unknown error!");
                return false;
        }
        yield();
    }
    
    Serial.println("No free slots!");
    return false;
}

int16_t enroll_finger(int16_t fid) {
    int16_t p = -1;
    Serial.println("Waiting for valid finger to enroll");
    while (p != FPM_OK) {
        p = finger.getImage();
        switch (p) {
            case FPM_OK:
                Serial.println("Image taken");
                break;
            case FPM_NOFINGER:
                Serial.println(".");
                break;
            case FPM_PACKETRECIEVEERR:
                Serial.println("Communication error");
                break;
            case FPM_IMAGEFAIL:
                Serial.println("Imaging error");
                break;
            case FPM_TIMEOUT:
                Serial.println("Timeout!");
                break;
            case FPM_READ_ERROR:
                Serial.println("Got wrong PID or length!");
                break;
            default:
                Serial.println("Unknown error");
                break;
        }
        yield();
    }
    // OK success!

    p = finger.image2Tz(1);
    switch (p) {
        case FPM_OK:
            Serial.println("Image converted");
            break;
        case FPM_IMAGEMESS:
            Serial.println("Image too messy");
            return p;
        case FPM_PACKETRECIEVEERR:
            Serial.println("Communication error");
            return p;
        case FPM_FEATUREFAIL:
            Serial.println("Could not find fingerprint features");
            return p;
        case FPM_INVALIDIMAGE:
            Serial.println("Could not find fingerprint features");
            return p;
        case FPM_TIMEOUT:
            Serial.println("Timeout!");
            return p;
        case FPM_READ_ERROR:
            Serial.println("Got wrong PID or length!");
            return p;
        default:
            Serial.println("Unknown error");
            return p;
    }

    Serial.println("Remove finger");
    delay(2000);
    p = 0;
    while (p != FPM_NOFINGER) {
        p = finger.getImage();
        yield();
    }

    p = -1;
    Serial.println("Place same finger again");
    while (p != FPM_OK) {
        p = finger.getImage();
        switch (p) {
            case FPM_OK:
                Serial.println("Image taken");
                break;
            case FPM_PACKETRECIEVEERR:
                Serial.println("Communication error");
                break;
        }
        yield();
    }

    // OK success!

    p = finger.image2Tz(2);
    switch (p) {
        case FPM_OK:
            Serial.println("Image converted");
            break;
        case FPM_IMAGEMESS:
            Serial.println("Image too messy");
            return p;
        case FPM_PACKETRECIEVEERR:
            Serial.println("Communication error");
            return p;
        case FPM_FEATUREFAIL:
            Serial.println("Could not find fingerprint features");
            return p;
        case FPM_INVALIDIMAGE:
            Serial.println("Could not find fingerprint features");
            return p;
        case FPM_TIMEOUT:
            Serial.println("Timeout!");
            return false;
        case FPM_READ_ERROR:
            Serial.println("Got wrong PID or length!");
            return false;
        default:
            Serial.println("Unknown error");
            return p;
    }


    // OK converted!
    p = finger.createModel();
    if (p == FPM_OK) {
        Serial.println("Prints matched!");
    } else if (p == FPM_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    } else if (p == FPM_ENROLLMISMATCH) {
        Serial.println("Fingerprints did not match");
        return p;
    } else if (p == FPM_TIMEOUT) {
        Serial.println("Timeout!");
        return p;
    } else if (p == FPM_READ_ERROR) {
        Serial.println("Got wrong PID or length!");
        return p;
    } else {
        Serial.println("Unknown error");
        return p;
    }

    Serial.print("ID "); Serial.println(fid);
    p = finger.storeModel(fid);
    if (p == FPM_OK) {
        Serial.println("Stored!");
        return 0;
    } else if (p == FPM_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    } else if (p == FPM_BADLOCATION) {
        Serial.println("Could not store in that location");
        return p;
    } else if (p == FPM_FLASHERR) {
        Serial.println("Error writing to flash");
        return p;
    } else if (p == FPM_TIMEOUT) {
        Serial.println("Timeout!");
        return p;
    } else if (p == FPM_READ_ERROR) {
        Serial.println("Got wrong PID or length!");
        return p;
    } else {
        Serial.println("Unknown error");
        return p;
    }
}


int deleteFingerprint(int fid) {
    int p = -1;

    p = finger.deleteModel(fid);

    if (p == FPM_OK) {
        Serial.println("Deleted!");
    } else if (p == FPM_PACKETRECIEVEERR) {
        Serial.println("Communication error");
        return p;
    } else if (p == FPM_BADLOCATION) {
        Serial.println("Could not delete in that location");
        return p;
    } else if (p == FPM_FLASHERR) {
        Serial.println("Error writing to flash");
        return p;
    } else if (p == FPM_TIMEOUT) {
        Serial.println("Timeout!");
        return p;
    } else if (p == FPM_READ_ERROR) {
        Serial.println("Got wrong PID or length!");
        return p;
    } else {
        Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
        return p;
    }
}


int search_database(void) {
    int16_t p = -1;

    /* first get the finger image */
    //Serial.println("Waiting for valid finger");
    while (p != FPM_OK) {
        p = finger.getImage();
        switch (p) {
            case FPM_OK:
              //  Serial.println("Image taken");
                break;
            case FPM_NOFINGER:
               // Serial.println(".");
                break;
            case FPM_PACKETRECIEVEERR:
            //    Serial.println("Communication error");
                break;
            case FPM_IMAGEFAIL:
             //   Serial.println("Imaging error");
                break;
            case FPM_TIMEOUT:
              //  Serial.println("Timeout!");
                break;
            case FPM_READ_ERROR:
              //  Serial.println("Got wrong PID or length!");
                break;
            default:
              //  Serial.println("Unknown error");
                break;
        }
        yield();
    }

    /* convert it */
    p = finger.image2Tz();
    switch (p) {
        case FPM_OK:
           // Serial.println("Image converted");
            break;
        case FPM_IMAGEMESS:
           // Serial.println("Image too messy");
            return p;
        case FPM_PACKETRECIEVEERR:
           // Serial.println("Communication error");
            return p;
        case FPM_FEATUREFAIL:
          //  Serial.println("Could not find fingerprint features");
            return p;
        case FPM_INVALIDIMAGE:
          //  Serial.println("Could not find fingerprint features");
            return p;
        case FPM_TIMEOUT:
          //  Serial.println("Timeout!");
            return p;
        case FPM_READ_ERROR:
           // Serial.println("Got wrong PID or length!");
            return p;
        default:
          //  Serial.println("Unknown error");
            return p;
    }

    p = 0;
    while (p != FPM_NOFINGER) {
        p = finger.getImage();
        yield();
    }
    

    /* search the database for the converted print */
    uint16_t fid, score;
    p = finger.searchDatabase(&fid, &score);
    if (p == FPM_OK) {
        Serial.println("Match");
    } else if (p == FPM_PACKETRECIEVEERR) {
    //    Serial.println("Communication error");
        return p;
    } else if (p == FPM_NOTFOUND) {
        Serial.println("Notmatch");
        Serial.println("Reintentar");
        return p;
    } else if (p == FPM_TIMEOUT) {
    //    Serial.println("Timeout!");
        return p;
    } else if (p == FPM_READ_ERROR) {
     //   Serial.println("Got wrong PID or length!");
        return p;
    } else {
     //   Serial.println("Unknown error");
        return p;
    }

    // found a match!
   Serial.println(fid);
}