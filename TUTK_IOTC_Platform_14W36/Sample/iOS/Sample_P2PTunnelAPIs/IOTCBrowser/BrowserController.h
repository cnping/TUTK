//
//  BrowserController.h
//  IOTCBrowser
//
//  Created by tutk on 3/5/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface BrowserController : UIViewController  {
    
    IBOutlet UIWebView *webBrowser;
    IBOutlet UITextField *UrlTextField;
    IBOutlet UITextField *PortTextField;
    IBOutlet UIButton *GoButton;
}

@property (nonatomic, retain) IBOutlet UIWebView *webBrowser;
@property (nonatomic, retain) IBOutlet UITextField *UrlTextField;
@property (nonatomic, retain) IBOutlet UITextField *PortTextField;
@property (nonatomic, retain) IBOutlet UIButton *GoButton;

@end
