// see here how to use this library: http://www.youtube.com/watch?v=kVw6kPSJJfw


#include "AnalogMatrixKeypad.h"
AnalogMatrixKeypad::AnalogMatrixKeypad(byte ap){
	analogPin = ap;
	debounceTime = 250;
	threshold = 4;
	numberOfKeys = 16;
}

void AnalogMatrixKeypad::setDebounceTime(unsigned int time){
	debounceTime = time;
}

void AnalogMatrixKeypad::setNumberOfKeys(unsigned int keys){
	numberOfKeys = keys;
}

void AnalogMatrixKeypad::setThresholdValue(byte tv){
	threshold = tv;
}

char AnalogMatrixKeypad::readKey(){
	char key = KEY_NOT_PRESSED;
	int aValue = analogRead(analogPin);
	if(numberOfKeys == 16){
		if((aValue > 0)&&(millis() -lastValue >= debounceTime)){
			if((aValue > (925 -threshold)) &&(aValue < (925 + threshold)))
				key = KEY_1;
			if((aValue > (907 -threshold)) &&(aValue < (907 + threshold)))
				key = KEY_2;
			if((aValue > (890 -threshold)) &&(aValue < (890 + threshold)))
				key = KEY_3;
			if((aValue > (873 -threshold)) &&(aValue < (873 + threshold)))
				key = KEY_A;
			if((aValue > (849 -threshold)) &&(aValue < (849 + threshold)))
				key = KEY_4;
			if((aValue > (834 -threshold)) &&(aValue < (834 + threshold)))
				key = KEY_5;
			if((aValue > (819 -threshold)) &&(aValue < (819 + threshold)))
				key = KEY_6;
			if((aValue > (805 -threshold)) &&(aValue < (805 + threshold)))
				key = KEY_B;
			if((aValue > (784 -threshold)) &&(aValue < (784 + threshold)))
				key = KEY_7;
			if((aValue > (771 -threshold)) &&(aValue < (771 + threshold)))
				key = KEY_8;
			if((aValue > (759 -threshold)) &&(aValue < (759 + threshold)))
				key = KEY_9;
			if((aValue > (746 -threshold)) &&(aValue < (746 + threshold)))
				key = KEY_C;
			if((aValue > (729 -threshold)) &&(aValue < (729 + threshold)))
				key = KEY_STAR;
			if((aValue > (717 -threshold)) &&(aValue < (717 + threshold)))
				key = KEY_0;
			if((aValue > (707 -threshold)) &&(aValue < (707 + threshold)))
				key = KEY_HASH;
			if((aValue > (696 -threshold)) &&(aValue < (696 + threshold)))
				key = KEY_D;
			lastValue = millis();
		}
	} else if(numberOfKeys == 4){
		if((aValue > 0)&&(millis() -lastValue >= debounceTime)){
			if((aValue > (340 -threshold)) &&(aValue < (340 + threshold)))
				key = KEY_1;
			if((aValue > (452 -threshold)) &&(aValue < (452 + threshold)))
				key = KEY_2;
			if((aValue > (231 -threshold)) &&(aValue < (231 + threshold)))
				key = KEY_3;
			if((aValue > (277 -threshold)) &&(aValue < (277 + threshold)))
				key = KEY_4;
			lastValue = millis();
			/*
			if((aValue > (520 -threshold)) &&(aValue < (520 + threshold)))
				key = KEY_1;
			if((aValue > (690 -threshold)) &&(aValue < (690 + threshold)))
				key = KEY_2;
			if((aValue > (350 -threshold)) &&(aValue < (350 + threshold)))
				key = KEY_3;
			if((aValue > (420 -threshold)) &&(aValue < (420 + threshold)))
				key = KEY_4;
			lastValue = millis();*/
		}
	} else if(numberOfKeys == 5){
		if((aValue > 0)&&(millis() -lastValue >= debounceTime)){
			if((aValue > (177 -threshold)) &&(aValue < (177 + threshold)))
				key = KEY_1;
			if((aValue > (317 -threshold)) &&(aValue < (317 + threshold)))
				key = KEY_2;
			if((aValue > (609 -threshold)) &&(aValue < (609 + threshold)))
				key = KEY_3;
			if((aValue > (839 -threshold)) &&(aValue < (839 + threshold)))
				key = KEY_4;
			if((aValue > (931 -threshold)) &&(aValue < (931 + threshold)))
				key = KEY_5;
			if((aValue > (408 -threshold)) &&(aValue < (408 + threshold)))
				key = KEY_1_2;
			if((aValue > (641 -threshold)) &&(aValue < (641 + threshold)))
				key = KEY_1_3;
			if((aValue > (846 -threshold)) &&(aValue < (846 + threshold)))
				key = KEY_1_4;
//			if((aValue > (932 -threshold)) &&(aValue < (932 + threshold)))
//				key = KEY_1_5;
			if((aValue > (673 -threshold)) &&(aValue < (673 + threshold)))
				key = KEY_2_3;
			if((aValue > (853 -threshold)) &&(aValue < (853 + threshold)))
				key = KEY_2_4;
//			if((aValue > (934 -threshold)) &&(aValue < (934 + threshold)))
//				key = KEY_2_5;
			if((aValue > (878 -threshold)) &&(aValue < (878 + threshold)))
				key = KEY_3_4;
//			if((aValue > (942 -threshold)) &&(aValue < (942 + threshold)))
//				key = KEY_3_5;
			if((aValue > (958 -threshold)) &&(aValue < (958 + threshold)))
				key = KEY_4_5;
			lastValue = millis();
		}
	}
	return key;
}

// see here how to use this library: http://www.youtube.com/watch?v=kVw6kPSJJfw
