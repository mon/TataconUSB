chrome.app.runtime.onLaunched.addListener(function() {
  chrome.app.window.create('window.html', {
    'innerBounds': {
      'width': 466,
      'height': 580
    }
  });
});