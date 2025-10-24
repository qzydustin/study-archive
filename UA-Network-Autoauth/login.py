import re
import requests
import argparse
import sys

from urllib.parse import urlparse, urljoin

# Initial settings
initial_url = 'http://neverssl.com/'  # Any non-HTTPS URL to trigger redirect

# Global configuration
HEADERS = {
    'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/18.1 Safari/605.1.15'
}

def get_portal_info(redirect_url):
    """Extract portal ID and origin information"""
    match = re.search(r'portal=([a-f0-9-]+)', redirect_url)
    if not match:
        print("Unable to extract portal ID")
        sys.exit(1)
    
    portal_id = match.group(1)
    parsed_url = urlparse(redirect_url)
    origin = f"{parsed_url.scheme}://{parsed_url.hostname}:{parsed_url.port}"
    
    return portal_id, origin

def check_session_cookies(session):
    """Verify required cookies"""
    required_cookies = ['token', 'APPSESSIONID', 'portalSessionId']
    missing = [cookie for cookie in required_cookies if not session.cookies.get(cookie)]
    if missing:
        print(f"Missing required cookies: {', '.join(missing)}")
        sys.exit(1)
    return {cookie: session.cookies.get(cookie) for cookie in required_cookies}

def main():
    # parse arguments
    parser = argparse.ArgumentParser(description='Login script with username and password')
    parser.add_argument('-u', '--username', required=True, help='Username for login')
    parser.add_argument('-p', '--password', required=True, help='Password for login')
    args = parser.parse_args()
    
    # use arguments
    username = args.username
    password = args.password
    
    session = requests.Session()
    
    try:
        # Step 1: Initial request
        response = session.get(initial_url, allow_redirects=True, headers=HEADERS)
        redirect_url = response.headers.get('Location')
        
        if not redirect_url or "portal" not in redirect_url:
            print("No redirect to login page detected")
            sys.exit(1)
            
        # Get login page
        response = session.get(redirect_url, headers=HEADERS)
        
        # Step 2: Extract required information
        portal_id, origin = get_portal_info(redirect_url)
        if not portal_id or not origin:
            return 1
            
        cookies = check_session_cookies(session)
        if not cookies:
            return 1
        
        # Construct login request
        login_url = urljoin(origin, '/portal/LoginSubmit.action?from=LOGIN')
        login_headers = {
            **HEADERS,
            'Content-Type': 'application/x-www-form-urlencoded; charset=UTF-8',
            'Referer': redirect_url,
            'Origin': origin,
            'X-Requested-With': 'XMLHttpRequest'
        }
        
        payload = {
            'token': cookies['token'],
            'portal': portal_id,
            'user.username': username,
            'user.password': password
        }

        # Step 3: Submit login
        login_response = session.post(login_url, data=payload, headers=login_headers)
        
        print("\nLogin Response:")
        print(f"Status Code: {login_response.status_code}, status code does not matter.")
        print("Reinsert the network cable and check the network")
            
    except requests.exceptions.ConnectionError as e:
        print(f"Connection error: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Unexpected error: {e}")
        sys.exit(1)
    
    return 0

if __name__ == "__main__":
    exit(main())
