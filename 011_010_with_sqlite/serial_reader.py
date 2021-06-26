# Reads the values measured by the Arduino board
import serial  # importing the pyserial package

import asyncio
import sqlite3

ARDUINO_PORT = "/dev/ttyUSB0"

TEMPERATURE = float()
CO = float()


async def main():
    try:
        ser = serial.Serial(ARDUINO_PORT)
        ser.flushInput()

        with sqlite3.connect("./values.sqlite3") as db:
            cursor = db.cursor()
            table_exists = cursor.execute("SELECT name FROM sqlite_master WHERE type='table' AND name='values';")
            if table_exists.fetchone() is None:
                cursor.execute("CREATE TABLE \"values\" ("
                            "\"id\"	INTEGER NOT NULL,"
                            "\"timestamp\"	INTEGER NOT NULL DEFAULT (datetime('now', 'localtime')),"
                            "\"temperature\"	REAL NOT NULL,"
                            "\"co\"	REAL NOT NULL,"
                            "PRIMARY KEY(\"id\" AUTOINCREMENT)"
                            ");")
        while True:
            # reading the temperature value
            input_bytes = ser.readline()
            global TEMPERATURE
            TEMPERATURE = float(input_bytes[0:len(input_bytes) - 2].decode("utf-8"))
            # reading the CO value
            input_bytes = ser.readline()
            global CO
            CO = float(input_bytes[0:len(input_bytes) - 2].decode("utf-8"))

            write_data_to_db()
    except KeyboardInterrupt:
        print("bye")


def write_data_to_db():
    with sqlite3.connect("./values.sqlite3") as db:
        cursor = db.cursor()
        cursor.execute("INSERT INTO `values` (temperature, co) VALUES (?, ?)", (TEMPERATURE, CO))


asyncio.run(main())
