# UA-Network-Autoauth

A Python script designed to automate the login process for the University of Arizona's campus network, particularly for embedded devices. This script bypasses the need for a browser-based login by directly submitting the required credentials to the portal.

## Features

- Automates the login process for both wired and wireless connections to the University of Arizona's campus network.
- Replaces the browser-based login flow, making it ideal for embedded devices or IoT devices that lack a web browser.
- Handles cookie-based authentication required by the UA campus portal.

## Requirements

- Python 3.x
- `requests` library (install via `pip`)

### Install Dependencies

To install the required dependencies, run:

```bash
pip install requests
```

## Usage

Run the script from the command line with the following arguments:

```bash
python login.py -u <your_username> -p <your_password>
```

### Arguments

- `-u` or `--username`: Your University of Arizona username.
- `-p` or `--password`: Your University of Arizona password.

### Example

```bash
python login.py -u dustin -p mypassword123
```

This command will automatically log you into the University of Arizona's campus network.

## How it Works

1. **Initial Request**: The script first sends a request to a non-HTTPS page (`http://neverssl.com/`) to trigger a redirect to the campus portal.
2. **Redirect and Session Setup**: The script follows the redirect to the login page and extracts necessary information such as the `portal ID` and session cookies.
3. **Login Submission**: The script submits the login credentials along with necessary session cookies to the portal's login endpoint.
4. **Completion**: Once the login request is successful, you can reconnect your device to the network.

## Troubleshooting

- If the script does not work or you encounter connection issues, ensure that your device is connected to the campus network.
- Ensure that your credentials are correct.
