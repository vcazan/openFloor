
package {
	import flash.display.*;
	import flash.events.*;
	import flash.net.XMLSocket;
	
	public class openFloor extends Sprite {
		private var hostName:String = "localhost";
		private var port:uint = 9090;
		private var socket:XMLSocket;
		
		private var realX = new Array(10);
		private var realY = new Array(10);
		private var txtOut = new Array(6);

		private var circle:MovieClip = new track() as MovieClip;	

		public function openFloor() {
			socket = new XMLSocket();
			configureListeners(socket);
			socket.connect(hostName, port);
			
		}
		
		public function send(data:Object):void {
			socket.send(data);
		}
		
		private function configureListeners(dispatcher:IEventDispatcher):void {
			dispatcher.addEventListener(Event.CLOSE, closeHandler);
			dispatcher.addEventListener(Event.CONNECT, connectHandler);
			dispatcher.addEventListener(DataEvent.DATA, dataHandler);
			dispatcher.addEventListener(IOErrorEvent.IO_ERROR, ioErrorHandler);
			dispatcher.addEventListener(ProgressEvent.PROGRESS, progressHandler);
			dispatcher.addEventListener(SecurityErrorEvent.SECURITY_ERROR, securityErrorHandler);
		}
		
		private function closeHandler(event:Event):void {
			trace("closeHandler: " + event);
		}
		
		private function connectHandler(event:Event):void {
			trace("connectHandler: " + event);
		}
		
		private function dataHandler(event:DataEvent):void {
			processData(event.data);
		}
		
		private function ioErrorHandler(event:IOErrorEvent):void {
			trace("ioErrorHandler: " + event);
		}
		
		private function progressHandler(event:ProgressEvent):void {
			trace("progressHandler loaded:" + event.bytesLoaded + " total: " + event.bytesTotal);
		}
		
		private function securityErrorHandler(event:SecurityErrorEvent):void {
			trace("securityErrorHandler: " + event);
		}
		
		private function processData(message:String){

			txtOut = message.split("|");
			trace(txtOut);
			
			if (txtOut[0] == 0){
				stage.addChild(circle);
			}
			
			if (txtOut[0] == 1){
				realX = txtOut[2] *550;
				realY = txtOut[3] *400;
			}
			
			if (txtOut[0] == 2){
				stage.removeChild(circle);
			}
			
			circle.x = realX;
			circle.y = realY;
		}
	}
}