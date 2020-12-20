# Reads the values measured by the Arduino board
import serial  # importing the pyserial package
import pandas as pd
import plotly.express as px

import asyncio
import datetime
import csv
import logging

ARDUINO_PORT = "/dev/ttyUSB0"
# the number of minutes after the plot will be updated
PLOT_INTERVAL = 30

async def main():
    ser = serial.Serial(ARDUINO_PORT)
    ser.flushInput()

    logging.basicConfig(filename="./plots.log", filemode="a", encoding="utf-8",
                        format='%(asctime)s - %(name)s - %(levelname)s - %(message)s', level=logging.INFO)

    count = 1
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

        # updating the plot
        if count == PLOT_INTERVAL:
            await update_temperature_plot(now)
            await update_co_plot(now)
            count = 1
        else:
            count += 1

async def update_temperature_plot(updated_at):
    started_at = datetime.datetime.now()
    data = pd.read_csv("temperature_data.csv", names=["Time", "Temperature"])
    x_values = data["Time"]
    y_values = data["Temperature"]
    fig = px.scatter(x=x_values, y=y_values, title=f"Hőmérséklet az idő függvényében (frissítve: {updated_at})")
    fig.write_html("temperature_plot.html")
    time_taken = datetime.datetime.now() - started_at
    logging.info(f"Finished building temperature plot in {time_taken}")

async def update_co_plot(updated_at):
    started_at = datetime.datetime.now()
    data = pd.read_csv("co_data.csv", names=["Time", "Concentration"])
    x_values = data["Time"]
    y_values = data["Concentration"]
    fig = px.scatter(x=x_values, y=y_values, title=f"CO koncentráció az idő függvényében (frissítve: {updated_at})")
    fig.write_html("co_plot.html")
    time_taken = datetime.datetime.now() - started_at
    logging.info(f"Finished building CO plot in {time_taken}")

asyncio.run(main())
