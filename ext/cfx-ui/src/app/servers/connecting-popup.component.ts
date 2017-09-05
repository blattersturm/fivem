import { Component, OnInit } from '@angular/core';
import { GameService } from '../game.service';
import { Translation, TranslationService } from 'angular-l10n';

@Component({
	moduleId:    module.id,
	selector:    'connecting-popup',
	templateUrl: 'connecting-popup.component.html',
	styleUrls:   ['connecting-popup.component.scss']
})
export class ConnectingPopupComponent extends Translation implements OnInit {
	showOverlay = false;
	overlayClosable = true;
	overlayTitle: string;
	overlayMessage: string;
	overlayMessageData = {};
	closeLabel = "#Servers_CloseOverlay";

	constructor(
		private gameService: GameService,
		public translation: TranslationService
	) {
		super(translation);
	}

	ngOnInit() {
		this.gameService.connecting.subscribe(a => {
			this.overlayTitle = '#Servers_Connecting';
			this.overlayMessage = '#Servers_ConnectingTo';
			this.overlayMessageData = {serverName: a.address};
			this.showOverlay = true;
			this.overlayClosable = false;
		});

		this.gameService.connectFailed.subscribe(([server, message]) => {
			this.overlayTitle = '#Servers_ConnectFailed';
			this.overlayMessage = '#Servers_Message';
			this.overlayMessageData = {message};
			this.showOverlay = true;
			this.overlayClosable = true;
			this.closeLabel = "#Servers_CloseOverlay";
		});

		this.gameService.connectStatus.subscribe(a => {
			this.overlayTitle = '#Servers_Connecting';
			this.overlayMessage = '#Servers_Message';
			this.overlayMessageData = {message: a.message};
			this.showOverlay = true;
			this.overlayClosable = (a.count == 133 && a.total == 133); // magic numbers, yeah :(

			if (this.overlayClosable) {
				this.closeLabel = "#Servers_CancelOverlay";
			}
		});
	}

	closeOverlay() {
		this.showOverlay = false;

		this.gameService.cancelNativeConnect();
	}
}
