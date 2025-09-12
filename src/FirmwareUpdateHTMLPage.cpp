#include "../includes/FirmwareUpdateHTMLPage.hpp"

String HTML_FIRMWARE_NEW_UPDATE_PAGE = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <title>Smart Lock</title>
    <style>
        body {
            font-family: "Montserrat", sans-serif;
            background: #fafafa;
            margin: 0;
            display: flex;
            flex-direction: column;
            min-height: 100vh;
            color: #2c2c2c;
            -webkit-font-smoothing: antialiased;
            -moz-osx-font-smoothing: grayscale;
        }

        nav {
            background: white;
            box-shadow: 0 1px 6px rgb(0 0 0 / 0.1);
            padding: 1rem 2rem;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }

        nav h1 {
            margin: 0;
            font-weight: 700;
            font-size: 1.5rem;
            letter-spacing: 1px;
        }

        nav ul {
            display: flex;
            list-style: none;
            padding: 0;
            margin: 0;
        }

        nav ul li {
            margin-left: 1.5rem;
        }

        nav ul li a {
            text-decoration: none;
            color: #444;
            font-weight: 500;
            padding: 0.35rem 0.6rem;
            border-radius: 4px;
            transition: background-color 0.25s ease, color 0.25s ease;
            font-size: 0.9rem;
        }

        nav ul li a:hover {
            background-color: #eaeaea;
            color: #222;
        }

        nav ul li:last-child a {
            background-color: #d9534f;
            color: #fff;
            font-weight: 600;
            transition: background-color 0.3s ease;
        }

        nav ul li:last-child a:hover {
            background-color: #c9302c;
        }

        main {
            flex: 1;
            max-width: 600px;
            margin: 3rem auto 4rem;
            padding: 0 1.5rem;
            display: flex;
            flex-direction: column;
            gap: 1.8rem;
        }

        .update-info {
            font-weight: 600;
            font-size: 1.1rem;
            color: #222;
            border-left: 3px solid #1e90ff;
            padding-left: 0.8rem;
        }

        .update-warning {
            font-size: 0.95rem;
            color: #a33a3a;
            font-weight: 500;
            line-height: 1.4;
            border-left: 3px solid #e55353;
            padding-left: 0.8rem;
            user-select: none;
        }

        h2 {
            font-weight: 700;
            font-size: 1.3rem;
            margin-bottom: 0.6rem;
            color: #222;
        }

        .changelog-wrapper {
            position: relative;
            max-height: 7.5em;
            min-height: 5em;
            overflow: hidden;
            transition: max-height 0.35s ease;
            color: #444;
            line-height: 1.5;
            font-size: 1rem;
        }

        @media (max-width: 600px) {
            .changelog-wrapper {
                max-height: 7.5em;
                min-height: 5em;
            }
        }

        .changelog-wrapper.blur::after {
            content: "";
            position: absolute;
            bottom: 0;
            left: 0;
            width: 100%;
            height: 2.5em;
            pointer-events: none;
            background: linear-gradient(to bottom, rgba(250, 250, 250, 0), #fafafa);
            transition: opacity 0.3s ease;
            opacity: 1;
        }

        .changelog-wrapper.no-blur::after {
            opacity: 0;
        }

        input#toggleReadMore {
            display: none;
        }

        input#toggleReadMore:checked~.changelog-wrapper {
            max-height: 9999px;
            overflow: auto;
        }

        input#toggleReadMore:checked~.changelog-wrapper.blur::after {
            opacity: 0;
        }

        .readmore-container {
            display: flex;
            justify-content: flex-end;
            margin-top: 0.4rem;
            margin-bottom: 1rem;
        }

        label.readmore-label {
            font-size: 0.85rem;
            font-weight: 600;
            color: #1e90ff;
            cursor: pointer;
            user-select: none;
            transition: color 0.25s ease;
            padding: 0 0.3rem;
        }

        label.readmore-label:hover {
            color: #104e8b;
        }

        small.released-at {
            color: #888;
            font-size: 0.85rem;
            margin-top: 0.8rem;
            margin-bottom: 1.6rem;
        }

        .download-button {
            background-color: #1e90ff;
            color: white;
            font-weight: 700;
            font-size: 1.1rem;
            border: none;
            border-radius: 6px;
            padding: 0.9rem 2.6rem;
            cursor: pointer;
            align-self: center;
            box-shadow: 0 5px 15px rgb(30 144 255 / 0.35);
            transition: background-color 0.3s ease, box-shadow 0.3s ease;
            text-decoration: none;
            user-select: none;
        }

        .download-button:hover {
            background-color: #1669c1;
            box-shadow: 0 7px 20px rgb(22 105 193 / 0.6);
        }

        footer {
            background: #222;
            color: #ccc;
            text-align: center;
            padding: 1rem 1.5rem;
            font-size: 0.85rem;
            letter-spacing: 0.03em;
            margin-top: auto;
            user-select: none;
        }

        footer a {
            color: #6ca0dc;
            text-decoration: none;
            font-weight: 600;
            border-bottom: 1.5px solid transparent;
            transition: border-color 0.3s ease, color 0.3s ease;
        }

        footer a:hover,
        footer a:focus {
            color: #4991e2;
            border-bottom-color: #4991e2;
            outline: none;
        }
    </style>
</head>

<body>
    <nav>
        <h1>Smart Lock</h1>
        <ul>
            <li><a href="/users">Add Users</a></li>
            <li><a href="/home-wifi">Configure Home Wi-Fi</a></li>
            <li><a href="/device-password">Change Password</a></li>
            <li><a href="/device/reboot" style="background-color:#d9534f;color:#fff;">Restart Device</a></li>
        </ul>
    </nav>

    <main>
        <div class="update-info">
            You’re upgrading from <strong>[[currentVersion]]</strong> to <strong>[[newVersion]]</strong>.
        </div>

        <div class="update-warning" role="alert">
            Keep your device plugged in and connected to Wi-Fi during the update. Any disconnections or power loss may cause failure. Updates
            include security patches, bug fixes, and new features.
        </div>

        <input type="checkbox" id="toggleReadMore" />

        <div class="changelog-wrapper blur" id="changelogWrapper">
            <div class="changelog" id="changelog">
                [[changeLog]]
            </div>
        </div>

        <div class="readmore-container" id="readMoreContainer">
            <label for="toggleReadMore" class="readmore-label" id="readMoreLabel">Read More</label>
        </div>

        <small class="released-at">Released on [[releasedDate]]</small>

        <small style="text-align: center; display: block; color: #666; margin-top: -1rem; margin-bottom: 1rem; font-size: 0.9rem;">
            File size: [[downloadFileSize]]
        </small>

        <a class="download-button" href="/device/update/download" role="button">Download Update</a>
    </main>

    <footer>
        &copy; CUSIT Makerspace R&D Lab - City University of Science and Information Technology, Peshawar
        <br />
        <small>OTA Updates Powered by</small>
        <small>
            <a href="https://voyagerota.com/" target="_blank" rel="noopener noreferrer" style="color:#4a90e2; font-weight:600; text-decoration:none; margin-top:6px; display:inline-block; letter-spacing:0.05em;">
                Voyager OTA
            </a>
        </small>
    </footer>

    <script>
        document.addEventListener("DOMContentLoaded", () => {
            const changelogWrapper = document.getElementById("changelogWrapper");
            const changelog = document.getElementById("changelog");
            const readMoreContainer = document.getElementById("readMoreContainer");
            const toggleReadMore = document.getElementById("toggleReadMore");
            const readMoreLabel = document.getElementById("readMoreLabel");

            function checkOverflow() {
                const isOverflowing = changelog.scrollHeight > changelogWrapper.clientHeight;

                if (isOverflowing) {
                    readMoreContainer.style.display = "flex";
                    changelogWrapper.classList.add("blur");
                    toggleReadMore.checked = false;
                    readMoreLabel.textContent = "Read More";
                } else {
                    readMoreContainer.style.display = "none";
                    changelogWrapper.classList.remove("blur");
                }
            }

            setTimeout(checkOverflow, 100);

            toggleReadMore.addEventListener("change", () => {
                if (toggleReadMore.checked) {
                    readMoreLabel.textContent = "Read Less";
                    changelogWrapper.classList.remove("blur");
                } else {
                    readMoreLabel.textContent = "Read More";
                    if (changelog.scrollHeight > changelogWrapper.clientHeight) {
                        changelogWrapper.classList.add("blur");
                    }
                }
            });

            window.addEventListener("resize", () => {
                checkOverflow();
            });
        });
    </script>
</body>

</html>
)rawliteral";

String HTML_NO_INTERNET_UPDATE_PAGE = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>Smart Lock</title>
  <style>
    body {
      font-family: "Montserrat", sans-serif;
      background: #fafafa;
      margin: 0;
      display: flex;
      flex-direction: column;
      min-height: 100vh;
      color: #2c2c2c;
    }
    nav {
      background: white;
      box-shadow: 0 1px 6px rgba(0, 0, 0, 0.1);
      padding: 1rem 2rem;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    nav h1 {
      margin: 0;
      font-weight: 700;
      font-size: 1.5rem;
    }
    nav ul {
      display: flex;
      list-style: none;
      padding: 0;
      margin: 0;
    }
    nav ul li {
      margin-left: 1.5rem;
    }
    nav ul li a {
      text-decoration: none;
      color: #444;
      font-weight: 500;
      padding: 0.35rem 0.6rem;
      border-radius: 4px;
      transition: 0.25s;
      font-size: 0.9rem;
    }
    nav ul li a:hover {
      background-color: #eaeaea;
      color: #222;
    }
    nav ul li:last-child a {
      background-color: #d9534f;
      color: #fff;
      font-weight: 600;
    }
    nav ul li:last-child a:hover {
      background-color: #c9302c;
    }
    main {
      flex: 1;
      max-width: 600px;
      margin: 3rem auto;
      padding: 0 1.5rem;
      text-align: center;
    }
    .status-box {
      background: #f8d7da;
      border-left: 4px solid #dc3545;
      padding: 1.2rem;
      font-size: 1.05rem;
      border-radius: 6px;
      color: #721c24;
    }
    footer {
      background: #222;
      color: #ccc;
      text-align: center;
      padding: 1rem 1.5rem;
      font-size: 0.85rem;
    }
    footer a {
      color: #6ca0dc;
      font-weight: 600;
      text-decoration: none;
    }
    footer a:hover {
      color: #4991e2;
    }
  </style>
</head>
<body>
  <nav>
    <h1>Smart Lock</h1>
    <ul>
      <li><a href="/users">Add Users</a></li>
      <li><a href="/home-wifi">Configure Home Wi-Fi</a></li>
      <li><a href="/device-password">Change Password</a></li>
      <li><a href="/device/reboot">Restart Device</a></li>
    </ul>
  </nav>

  <main>
    <div class="status-box">
      <strong>No Internet Connection</strong><br>
      Unable to check for updates.<br>
      Current version: <strong>[[currentVersion]]</strong>
    </div>
  </main>

  <footer>
    &copy; CUSIT Makerspace R&D Lab - City University of Science and Information Technology, Peshawar
    <br />
    <small>OTA Updates Powered by <a href="https://voyagerota.com/" target="_blank">Voyager OTA</a></small>
  </footer>
</body>
</html>
)rawliteral";

String HTML_FIRMWARE_NO_UPDATE_PAGE = R"rawliteral(
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <title>Smart Lock</title>
    <style>
        body {
            font-family: "Montserrat", sans-serif;
            background: #fafafa;
            margin: 0;
            display: flex;
            flex-direction: column;
            min-height: 100vh;
            color: #2c2c2c;
            -webkit-font-smoothing: antialiased;
            -moz-osx-font-smoothing: grayscale;
        }

        nav {
            background: white;
            box-shadow: 0 1px 6px rgb(0 0 0 / 0.1);
            padding: 1rem 2rem;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }

        nav h1 {
            margin: 0;
            font-weight: 700;
            font-size: 1.5rem;
            letter-spacing: 1px;
        }

        nav ul {
            display: flex;
            list-style: none;
            padding: 0;
            margin: 0;
        }

        nav ul li {
            margin-left: 1.5rem;
        }

        nav ul li a {
            text-decoration: none;
            color: #444;
            font-weight: 500;
            padding: 0.35rem 0.6rem;
            border-radius: 4px;
            transition: background-color 0.25s ease, color 0.25s ease;
            font-size: 0.9rem;
        }

        nav ul li a:hover {
            background-color: #eaeaea;
            color: #222;
        }

        nav ul li:last-child a {
            background-color: #d9534f;
            color: #fff;
            font-weight: 600;
            transition: background-color 0.3s ease;
        }

        nav ul li:last-child a:hover {
            background-color: #c9302c;
        }

        main {
            flex: 1;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 2rem;
            text-align: center;
        }

        .no-updates-container {
            background: #ffffff;
            border: 1px solid #e0e0e0;
            border-radius: 12px;
            padding: 2rem 2.5rem;
            max-width: 400px;
            box-shadow: 0 6px 20px rgba(0, 0, 0, 0.05);
        }

        .icon-success {
            font-size: 2.5rem;
            margin-bottom: 1rem;
        }

        .no-updates-container h2 {
            font-size: 1.4rem;
            font-weight: 700;
            color: #2c2c2c;
            margin-bottom: 0.5rem;
        }

        .no-updates-container p {
            font-size: 1rem;
            color: #444;
        }

        footer {
            background: #222;
            color: #ccc;
            text-align: center;
            padding: 1rem 1.5rem;
            font-size: 0.85rem;
            letter-spacing: 0.03em;
            margin-top: auto;
        }

        footer a {
            color: #6ca0dc;
            text-decoration: none;
            font-weight: 600;
            border-bottom: 1.5px solid transparent;
            transition: border-color 0.3s ease, color 0.3s ease;
        }

        footer a:hover,
        footer a:focus {
            color: #4991e2;
            border-bottom-color: #4991e2;
            outline: none;
        }

        @media (max-width: 600px) {
            nav {
                flex-direction: column;
                align-items: flex-start;
            }

            nav ul {
                flex-direction: column;
                gap: 0.6rem;
                margin-top: 1rem;
            }

            .no-updates-container {
                width: 100%;
                padding: 1.5rem;
            }
        }
    </style>
</head>

<body>
    <nav>
        <h1>Smart Lock</h1>
        <ul>
            <li><a href="/users">Add Users</a></li>
            <li><a href="/home-wifi">Configure Home Wi-Fi</a></li>
            <li><a href="/device-password">Change Password</a></li>
            <li><a href="/device/reboot" style="background-color:#d9534f;color:#fff;">Restart Device</a></li>
        </ul>
    </nav>

    <main>
        <div class="no-updates-container">
            <h2>No Updates Available</h2>
            <p>Your device is already running the latest firmware version <strong>[[currentVersion]]</strong>.</p>
        </div>
    </main>

    <footer>
        &copy; CUSIT Makerspace R&D Lab - City University of Science and Information Technology, Peshawar
        <br />
        <small>OTA Updates Powered by</small>
        <small>
            <a href="https://voyagerota.com/" target="_blank" rel="noopener noreferrer" style="color:#4a90e2; font-weight:600; text-decoration:none; margin-top:6px; display:inline-block; letter-spacing:0.05em;">
                Voyager OTA
            </a>
        </small>
    </footer>
</body>

</html>
)rawliteral";