#pragma once

const int NUM_FLOORS = 25;	//предел [2, 99] этажей
const int NUM_CARS = 4;		//предео [1, 9] кабинок
//const int HIGHEST_FLOOR = 20;
//const int LOWEST_FLOR = 1;

const int STANDING_TIME = 5000; //in milliseconds
const int MOVING_TIME = 1600;
const int ACCELERATION_TIME = 3000;
const int SLOW_MOVING_TIME = 4000;
const int OPENING_DOOR_TIME = 1000;
const int CLOSING_DOOR_TIME = 1000;

const int TIME_INTERVAL = 200;

const char CALL_LOG_FILE_NAME[] = "elevator_log.txt";
const char LOG_READER_NAME[] = "Debug\\LogReader.exe";