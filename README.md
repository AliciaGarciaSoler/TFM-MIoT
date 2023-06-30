# TFM-MIoT
This repository holds the different elements that create my master's dissertation. There are three folders in this repository:
  1. shedCode: This folder contains the .ino file that needs to be run on the *Shed*.
  2. dashboard: this folder contains the code that composes the *Dashboard*.
  3. teacherApp: This folder contains both the Android app's apk and the code for the *Teacher's App*.
## shedCode

Inside this folder, you will find a **.ino** file with the same name. This file should be run on the hardware setup as explained in the accompanying project document.

## dashboard

This folder contains a **JSON** file for creating a dashboard and the corresponding Node-RED flow. To implement this part of the project, you need to have Node-RED installed on your machine. Follow the [official documentation](https://nodered.org/docs/getting-started/local) for information on installing Node-RED.

To use the dashboard:

1. Import the JSON file into Node-RED.
2. Deploy the imported flow.
3. Access the dashboard to view and interact with the project data.

## teachersApp

Inside this folder, you will find two items:

1. An **APK** file that you can download and install on your Android smartphone.
2. A **ZIP** folder containing the code that supports the app. To view the code, do not extract the files; instead, open the ZIP folder using the Android Studio IDE.

When downloading the app via the APK file, your smartphone may mark it as potentially dangerous or prompt you for installation permission. Proceed with the installation as required.
