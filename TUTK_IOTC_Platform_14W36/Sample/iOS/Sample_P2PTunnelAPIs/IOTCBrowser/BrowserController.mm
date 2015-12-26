//
//  BrowserController.m
//  IOTCBrowser
//
//  Created by tutk on 3/5/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "BrowserController.h"
#import "AppDelegate.h"

@implementation BrowserController

@synthesize webBrowser;
@synthesize UrlTextField;
@synthesize PortTextField;
@synthesize GoButton;

#pragma mark - View lifecycle

- (IBAction) GoPressed
{
    NSString *str;
    
    AppDelegate *delegate = [[UIApplication sharedApplication] delegate];
    int r = [delegate GetTunnelStatus];
    if( r < 0)
    {
        if(r == -40000)
            str = @"Tunnel is starting...";
        else
            str = [NSString stringWithFormat:@"Tunnel not ready : %d", [delegate GetTunnelStatus]];
        
        UIAlertView *alert = [[UIAlertView alloc] initWithTitle:@"Tunnel Error"
                                                        message:str
                                                       delegate:nil
                                              cancelButtonTitle:@"OK"
                                              otherButtonTitles:nil];
        
        [alert show];
    }
    
    [PortTextField setText:[NSString stringWithFormat:@"%d", [delegate GetMappedPort]]];
    
    NSMutableString *url_str;
    url_str = [NSMutableString stringWithFormat:@"http://127.0.0.1:%d", [delegate GetMappedPort]];
    [url_str appendString:[UrlTextField text]];
    
    NSLog(@"Connect to url : %@", url_str);
    
    NSURL *url = [NSURL URLWithString:url_str];
    NSURLRequest *request = [NSURLRequest requestWithURL:url
                                             cachePolicy:NSURLRequestReloadIgnoringCacheData
                                         timeoutInterval:10.0];

    [webBrowser loadRequest:request];
}

- (void)viewDidLoad {

    [super viewDidLoad];
}

- (void)viewDidAppear:(BOOL)animated
{
    NSLog(@"viewDidAppear");
    [UrlTextField setText:@"/harry.html"];
}

- (void)viewDidUnload {
    
    [super viewDidUnload];
}

@end
