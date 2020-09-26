class Service {
    static async post(data) {
        const url = '/Arduino';
        const response = await fetch(url, {
            method: 'POST', // *GET, POST, PUT, DELETE, etc.
            mode: 'cors', // no-cors, *cors, same-origin
            cache: 'no-cache', // *default, no-cache, reload, force-cache, only-if-cached
            credentials: 'same-origin', // include, *same-origin, omit
            headers: {
              'Content-Type': 'application/json'
              // 'Content-Type': 'application/x-www-form-urlencoded',
            },
            redirect: 'follow', // manual, *follow, error
            referrerPolicy: 'no-referrer', // no-referrer, *no-referrer-when-downgrade, origin, origin-when-cross-origin, same-origin, strict-origin, strict-origin-when-cross-origin, unsafe-url
            body: JSON.stringify(data) // body data type must match "Content-Type" header
          });
          return true; 
    }
}

class Helper {
    static componentToHex(c) {
        var hex = c.toString(16);
        return hex.length == 1 ? "0" + hex : hex;
    }
      
    static rgbToHex(r, g, b) {
        return "#" + componentToHex(r) + componentToHex(g) + componentToHex(b);
    }

    static hexToRgb(hex) {
        var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
        const r = `${parseInt(result[1], 16)}`.padStart(3, '0');
        const g = `${parseInt(result[2], 16)}`.padStart(3, '0');
        const b = `${parseInt(result[3], 16)}`.padStart(3, '0');

        return `${r}${g}${b}`;
    }
}

function changeColor(event) {
    const rgb = Helper.hexToRgb(event.target.value);
    Service.post({command: `#${rgb}`});
}

function preset(event) {
    if(event.target.value) {
        Service.post({command: `${event.target.value}`});
    }
}