
var Popup = {
    /* 
        * alert 弹窗 text 必传
        */
    alert: function(text) {
        var model = document.getElementById('popupMark');
        if (model) {
            var content = document.getElementById('alertText');
            content.innerText = text;
            model.style.display = 'block';
            return
        }
        var creatediv = document.createElement('div'); // 创建div
        creatediv.className = 'popup_mark'; // 添加class
        creatediv.setAttribute('id', 'popupMark'); // 添加ID
        var contentHtml = '<div class="popup_box">' +
            '<h1><img src="img/exclamation.png"/></h1>' +
            '<p id="alertText">' + text + '</p>' +
            '<h3 id="knowBtn">我知道了</h3>' +
            '<div class="closePopup" id="closePopup"></div>' +
            '</div>'
        creatediv.innerHTML = contentHtml;
        document.body.appendChild(creatediv);
        document.getElementById('closePopup').addEventListener('click', function() {
            Popup.sureAlert();
        })
        document.getElementById('knowBtn').addEventListener('click', function() {
            Popup.sureAlert();
        })
    },
    /* 
        * 确定弹窗
        */
    sureAlert: function() {
        var model = document.getElementById('popupMark');
        model.style.display = 'none'
    },
    confirm: function(text, fn) {
        var model = document.getElementById('popupConfirm');
        if (model) {
            var content = document.getElementById('confirmText');
            content.innerText = text;
            model.style.display = 'block';
            return
        }
        var creatediv = document.createElement('div'); // 创建div
        creatediv.className = 'popup_mark'; // 添加class
        creatediv.setAttribute('id', 'popupConfirm'); // 添加ID
        var contentHtml = '<div class="popup_box">' +
            '<h1><img src="http://www.jq22.com/tp/6370052847182084148113881.png"/></h1>' +
            '<p id="confirmText">' + text + '</p>' +
            '<div class="confirmBtn"><a id="yesConfirm" href="javascript:void(0)">确定</a><a id="cancelConfirm" href="javascript:void(0)">取消</a></div>' +
            '<div class="closePopup" id="closeConfirm"></div>' +
            '</div>'
        creatediv.innerHTML = contentHtml;
        document.body.appendChild(creatediv);
        document.getElementById('closeConfirm').addEventListener('click', function() {
            Popup.closeConfirm();
        })
        document.getElementById('cancelConfirm').addEventListener('click', function() {
            Popup.closeConfirm();
        })
        document.getElementById('yesConfirm').addEventListener('click', function() {
            Popup.sureConfirm(fn);
        })
    },
    closeConfirm: function() {
        var model = document.getElementById('popupConfirm');
        model.style.display = 'none'
    },
    sureConfirm: function(fn) {
        var model = document.getElementById('popupConfirm');
        model.style.display = 'none';
        if (typeof fn == 'function') {
            fn();
        }
    }
}
