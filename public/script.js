document.addEventListener('DOMContentLoaded', () => {
  const configDiv = document.getElementById('configTable');
  const saveButton = document.getElementById('saveButton');

  fetch('/api/cloche')
    .then(response => response.json())
    .then(data => {
      const table = document.createElement('table');
      const headersRow = document.createElement('tr');
      for (const prop of ['heure', 'creneau', 'fin', 'duree']) {
        const headerCell = document.createElement('th');
        headerCell.textContent = prop.charAt(0).toUpperCase() + prop.slice(1); 
        headersRow.appendChild(headerCell);
      }
      table.appendChild(headersRow);

      data.forEach(horaire => {
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

      configDiv.appendChild(table);
    })
    .catch(error => console.error('Error fetching configuration:', error));

    saveButton.addEventListener('click', () => {
      const tableRows = configDiv.querySelectorAll('tr');
      const horaires = [];
    
      tableRows.forEach((row, rowIndex) => {
        if (rowIndex !== 0) { 
          const cells = row.querySelectorAll('td');
          const horaire = {};
    
          cells.forEach((cell, cellIndex) => {
            const propName = cellIndex === 0 ? 'heure' : cellIndex === 1 ? 'creneau' : cellIndex === 2 ? 'fin' : 'duree';
            let value = cell.textContent.trim();
            horaire[propName] = value;
          });
    
          horaires.push(horaire);
        }
      });
      fetch('/api/cloche', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({ horaires })
      })
        .then(response => response.json())
        .then(data => console.log(data))
        .catch(error => console.error('Error saving configuration:', error));
    });
});