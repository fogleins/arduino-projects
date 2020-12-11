# Reads the values measured by the Arduino board
import serial  # importing the pyserial package
import pandas as pd
import plotly.express as px

import asyncio
import datetime
import csv

ARDUINO_PORT = "/dev/ttyUSB0"

async def main():
    ser = serial.Serial(ARDUINO_PORT)
    ser.flushInput()

    count = 0
    while True:
        now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        with open("temperature_data.csv", "a", newline="") as f:
            input_bytes = ser.readline()
            parsed_bytes = float(input_bytes[0:len(input_bytes)-2].decode("utf-8"))
            writer = csv.writer(f)
            writer.writerow([now, parsed_bytes])

        with open("co_data.csv", "a", newline="") as f:
            input_bytes = ser.readline()
            parsed_bytes = float(input_bytes[0:len(input_bytes)-2].decode("utf-8"))
            writer = csv.writer(f)
            writer.writerow([now, parsed_bytes])

        # we update the graph every ten minutes
        if count == 9:
            await update_temperature_plot(now)
            await update_co_plot(now)
            count = -1
        count += 1

async def update_temperature_plot(updated_at):
    data = pd.read_csv("temperature_data.csv", names=["Time", "Temperature"])
    x_values = data["Time"]
    y_values = data["Temperature"]
    fig = px.scatter(x=x_values, y=y_values, title=f"Hőmérséklet az idő függvényében (frissítve: {updated_at})")
    fig.write_html("temperature_plot.html")

async def update_co_plot(updated_at):
    data = pd.read_csv("co_data.csv", names=["Time", "Concentration"])
    x_values = data["Time"]
    y_values = data["Concentration"]
    fig = px.scatter(x=x_values, y=y_values, title=f"CO koncentráció az idő függvényében (frissítve: {updated_at})")
    fig.write_html("co_plot.html")

asyncio.run(main())
