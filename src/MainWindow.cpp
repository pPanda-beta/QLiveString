#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <thread>
#include <string>
#include <cstdio>
#include <regex>
#include <map>


using namespace std;

#define g qDebug()<<__LINE__;

const char* page = R"dhamak(
<html>
<head>		<title>Live String Demo</title>	</head>
<body>
	<div id="d1">Live String Example</div>
	<input id="mvcPort" type="number" placeholder="The port for xhr" value="5999"/>
	<textarea id="t1"  style="width:100%; height:70%" >Hello Dual(cpp-js) World</textarea>
	<script>
		var t1b = document.getElementById("t1"); 
		var failid,mode="get",retryInt=100; // tHe globals
		
		var xhr = new XMLHttpRequest();
		xhr.timeout = 7500;			//infinite timeout 
		function update(md,ri) {
			if(typeof md !=="undefined")	mode=md;
			if(typeof ri !=="undefined")	retryInt=ri;
			console.log("new ..."+mode+" "+retryInt+" : "+t1b.value);
			xhr.abort();
			clearTimeout(failid);
			var mvcLink = "//" + document.location.hostname + ":"+document.getElementById("mvcPort").value;
			xhr.open("GET", mvcLink + "/?mode="+mode+"&v=" + encodeURI(t1b.value));
			xhr.send(null);
		}
	
	
		xhr.onreadystatechange = function () {
			if (xhr.readyState === 4)
				if (xhr.status === 200) {
					t1b.value = xhr.responseText;
					console.log("suc ...");
					update("get",100);
				} else {
					console.log("fail ...");
					failid = setTimeout(update,parseInt(retryInt*=1.2));
				}
		};
	
		t1b.onkeyup = t1b.onkeydown = t1b.input = t1b.onchange = function(){	update("set",50);	};
		update("get",100);
	</script>
</body>
</html>
)dhamak";	

		

		
//'NONE OF UR BUSINESS' STARTS FROME HERE
		
string to_str(auto &&x)
{
	ostringstream os;
	os<<x;
	return os.str();
}		
auto http_res(auto &&msg)
{
	return  "HTTP/1.1 200 OK\r\nConnection: Close\r\n"
			"Access-Control-Allow-Origin: * \r\n"
			"Access-Control-Allow-Headers:GET,POST,PUT\r\n"
			"Content-Length: "+to_str(msg.size())+"\r\n"
			"Content-Type: text/html\r\n\r\n"+msg.data()+"\r\n";
}

template <typename T, typename R=vector<T>>
R tokenize(const T &arr, const regex& reg)
{	
	regex_token_iterator<decltype(begin(arr))> iter(begin(arr), end(arr), reg, -1),end;
	return R(iter,end);
}

template<typename T>
T deHexFilter(const T &v)
{
	T org_v;
	org_v.reserve(v.size());
	for(size_t i=0; i<v.size(); i++)
		if(v[i]!='%')
			org_v+=v[i];
		else
		{
			int c;
			sscanf(v.data()+i+1,"%2x",&c);
			org_v+=c;
			i+=2;
		}
	return org_v;
}

map<string,string> parseGET(const string &req)
{
	const static regex reg("[&]+");
	
	auto &&st=req.find_first_of('?')+1;
	auto &&en=req.find_last_of(' ');
	map<string,string> parsed_req;
	
	if(st==string::npos)
		return parsed_req;
	

	for(sregex_token_iterator iter(begin(req)+st,en==string::npos?end(req):begin(req)+en,reg,-1),end ; iter!=end; ++iter)
	{
		string &&t=*iter;
		
		auto m=t.find_first_of('=');
		if(m==string::npos || m>=t.size()-1)
			parsed_req[t.substr(0,m)]="";
		else
			parsed_req[t.substr(0,m)]=deHexFilter(t.substr(m+1));
	}
	return parsed_req;
}

//'NONE OF UR BUSINESS' ENDS HERE


MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow), ser(this)
{
	ui->setupUi(this);
	

	qDebug()<<page;	
	static auto res=http_res(page+""s);
	
	connect(&ser,&QTcpServer::newConnection,[&]
	{
		QTcpSocket *incp=ser.nextPendingConnection();
		connect(incp,&QTcpSocket::readyRead,[&,incp]
		{
			while(incp->bytesAvailable())
				incp->readAll();
			
			incp->write(res.data(),res.size());
			incp->waitForBytesWritten();
			incp->close();
		});
	});
	

	
	connect(&liveStrXhrSer,&QTcpServer::newConnection,[&]
	{
		QTcpSocket *incp=liveStrXhrSer.nextPendingConnection();
		connect(incp,&QTcpSocket::readyRead,[&,incp]
		{
			QByteArray req;
			while(incp->bytesAvailable())
				req+=incp->readAll();
			auto it=req.indexOf("GET ");
			if(it>=0)
			{
				auto &&p_req=parseGET(string(req.begin()+it,req.begin()+req.indexOf('\n',it)));
				
				qDebug()<<p_req["mode"].data()<<","<<p_req["v"].data();
				
				if( p_req["v"]!=curr)
				{
					if(p_req["mode"]=="get")
					{
						unique_lock<mutex> lk(this->mt0);
						incp->write(dres.data(),dres.size());
						incp->waitForBytesWritten();
						incp->close();
						return;
					}
					else
					{
						QTextCursor &&carr=ui->tE1->textCursor();
						ui->tE1->setText(p_req["v"].data());
						ui->tE1->setTextCursor(carr);
						on_tE1_textChanged();
					}
				}
					
				unique_lock<mutex> lk2(this->mt);
				pending.push_back(incp);
			}
		});
	});
	
	
	ser.listen(QHostAddress::Any,(unsigned short)ui->sitePort->value());
	liveStrXhrSer.listen(QHostAddress::Any,(unsigned short)ui->xhrPort->value());
	dres=http_res("Hello Qt World"s);
	
	qDebug()<<ser.serverPort();
}


void MainWindow::on_tE1_textChanged()
{
	unique_lock<mutex> lk(mt0),lk2(mt);
	curr=ui->tE1->toPlainText().toStdString();
	auto val=dres=http_res((string &&)curr);
	
	for(QTcpSocket* &incp:pending)
		if(incp->isOpen())
			incp->write(val.data(),val.length());
	for(auto &incp:pending)
		if(incp->isOpen())
		{
			incp->waitForBytesWritten();
			incp->close();
		}
	pending.clear();
}

MainWindow::~MainWindow()
{
	delete ui;
}


void MainWindow::on_sitePort_valueChanged(double siteport)
{
	ser.close();
    ser.listen(QHostAddress::Any,(unsigned short)siteport);
}


void MainWindow::on_xhrPort_valueChanged(double xhrport)
{
	liveStrXhrSer.close();
    liveStrXhrSer.listen(QHostAddress::Any,(unsigned short)xhrport);
}

