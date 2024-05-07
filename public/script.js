document.addEventListener('DOMContentLoaded', () => {
  const configDiv = document.getElementById('configTable');
  const saveButton = document.getElementById('saveButton');
  const messageContainer = document.createElement('div');
  messageContainer.id = 'messageContainer';

  fetch('/api/cloche')
    .then(response => response.json())
    .then(data => {
      for (const day in data) {
        const dayTable = createDayTable(data[day], day);
        configDiv.appendChild(dayTable);
      }
    })
    .catch(error => console.error('Error fetching configuration:', error));

  saveButton.addEventListener('click', () => {
    const dayTables = configDiv.querySelectorAll('.day-table');
    const horaires = {};

    dayTables.forEach(dayTable => {
      const day = dayTable.getAttribute('data-day');
      horaires[day] = [];

      const tableRows = dayTable.querySelectorAll('tr');

      tableRows.forEach((row, rowIndex) => {
        if (rowIndex !== 0) {
          const cells = row.querySelectorAll('td');
          const horaire = {};

          cells.forEach((cell, cellIndex) => {
            const propName = cellIndex === 0 ? 'heure' : cellIndex === 1 ? 'creneau' : cellIndex === 2 ? 'fin' : 'duree';
            let value = cell.textContent.trim();
            horaire[propName] = value;
          });

          horaires[day].push(horaire);
        }
      });
    });
    fetch('/api/cloche', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({horaires})
    })
      .then(response => {
        if (!response.ok) {
          throw new Error('Network response was not ok'+response.status);
        }
        return response.json();
      })
      .then(data => {
        showMessage('Configuration saved successfully', 'success');
        console.log(data);
      })
      .catch(error => {
        showMessage('Error saving configuration', 'error');
        console.error('Error saving configuration:', error)
      });
  });

  document.body.appendChild(messageContainer);
});

function createDayTable(horaires, day) {
  const tableContainer = document.createElement('div'); 
  tableContainer.classList.add('table-container');
  
  const dayHeading = document.createElement('h2'); 
  dayHeading.textContent = day.charAt(0).toUpperCase() + day.slice(1);
  dayHeading.classList.add('day-heading');
  
  const table = document.createElement('table');
  table.classList.add('day-table');
  table.setAttribute('data-day', day);

  const headersRow = document.createElement('tr');
  for (const prop of ['heure', 'creneau', 'fin', 'duree']) {
    const headerCell = document.createElement('th');
    headerCell.textContent = prop.charAt(0).toUpperCase() + prop.slice(1); 
    headersRow.appendChild(headerCell);
  }
  table.appendChild(headersRow);

  horaires.forEach(horaire => {
    const row = document.createElement('tr');
    
    for (const prop in horaire) {
      const cell = document.createElement('td');
      let value = horaire[prop];
      if (prop === 'heure' && typeof value === 'number') {
        const hours = Math.floor(value);
        const minutes = Math.round((value - hours) * 60);
        value = `${hours < 10 ? '0' : ''}${hours}:${minutes < 10 ? '0' : ''}${minutes}`;
      }
      cell.textContent = value; 
      cell.setAttribute('contenteditable', 'true'); 
      row.appendChild(cell);
    }
    
    table.appendChild(row);
  });
  
  tableContainer.appendChild(dayHeading); 
  tableContainer.appendChild(table); 
  
  return tableContainer; 
}

function showMessage(message, type) {
  const messageContainer = document.getElementById('messageContainer');
  messageContainer.textContent = message;
  messageContainer.className = type;
}



