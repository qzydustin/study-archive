/**
 * This script will extract the student list from the Classlist page in D2L.
 * The script can be run in the browser console.
 * Then, the script will create a CSV file that can be downloaded.
 * The CSV file will contain the name and email of each student.
 *
 * This script will get all shown students on the webpage,
 * so make sure to set the number of students shown per page to the maximum.
 */

// Get the table element
const table = document.querySelector("#z_g");

// Get all the rows in the table
const rows = table.querySelectorAll(
  "tbody > tr:not(.d_gh.d2l-table-row-first)"
);

// Iterate through each row and extract data, only add to extractedData array if identity is "Student"
const extractedData = [];
rows.forEach((row, index) => {
  const nameElement = row.querySelector("th > a");
  const name = nameElement ? nameElement.textContent.trim() : "N/A";

  const emailElement = row.querySelector("td:nth-child(4)");
  const email = emailElement ? emailElement.textContent.trim() : "N/A";

  // Get the identity field
  const identityElement = row.querySelector("td:nth-child(5)");
  const identity = identityElement ? identityElement.textContent.trim() : "N/A";

  // Only add data to the extractedData array when identity is "Student"
  if (identity === "Student") {
    extractedData.push([name, email]);
  }
});

// Sort the extractedData array based on the email (second column)
// extractedData.sort((a, b) => a[1].localeCompare(b[1]));

// Create a CSV file
function createCSV(data) {
  const csvContent =
    "Name,Email\n" + data.map((row) => `"${row[0]}",${row[1]}`).join("\n");
  const blob = new Blob([csvContent], { type: "text/csv" });
  const url = URL.createObjectURL(blob);

  // Create a download link
  const a = document.createElement("a");
  a.href = url;
  a.download = "student-list.csv";

  // Simulate a user click to download the file
  a.click();
}

// Call the function to create the CSV file
createCSV(extractedData);
