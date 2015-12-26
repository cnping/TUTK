//
//  AppDelegate.m
//  IOTCBrowser
//
//  Created by tutk on 3/5/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "AppDelegate.h"
#import "P2PTunnelAPIs.h"

@implementation AppDelegate

@synthesize window = _window;
@synthesize rootController = _rootController;
@synthesize managedObjectContext = __managedObjectContext;
@synthesize managedObjectModel = __managedObjectModel;
@synthesize persistentStoreCoordinator = __persistentStoreCoordinator;

typedef struct st_AuthData
{
	char szUsername[64];
	char szPassword[64];
} sAuthData;

int gPortMappingIndex;
int gSID;
int gTunnelStatus;
int gMappedPort;

- (void) StartTunnel
{
    gTunnelStatus = -40000;
    
    [UIApplication sharedApplication].networkActivityIndicatorVisible = YES;
    
    
    sAuthData authData;
	strcpy(authData.szUsername, "Tutk.com");
	strcpy(authData.szPassword, "P2P Platform");
    
    P2PTunnelAgentInitialize(4);
    
    int nErrFromDevice;
    int gSID = P2PTunnelAgent_Connect("E7KU997WPBB4UN6GYHFJ", (void *)&authData, sizeof(sAuthData), &nErrFromDevice);

    
    if(gSID > -1)
    {
        NSLog(@"P2PTunnelAgent_Connect success!");
        
        NSLog(@"Start to find a port to map.");
        gPortMappingIndex = -1;
        for(int i = 0; i < 100; i++) //try 100 ports for mapping.
        {
            gMappedPort = 10080 + i;
            
            gPortMappingIndex = P2PTunnelAgent_PortMapping(gSID, gMappedPort, 80);
            if(gPortMappingIndex < 0)
            {
                NSLog(@"P2PTunnelAgent_PortMapping %d failed : %d", gMappedPort, gPortMappingIndex);
            }
            else
            {
                break;
            }
        }
        NSLog(@"End port mapping, gMappedPort = %d", gMappedPort);
        
        if(gPortMappingIndex > 0)
        {
            gTunnelStatus = gSID;
        }
        else
            gTunnelStatus = gPortMappingIndex;
    }
    else
    {
        NSLog(@"P2PTunnelAgent_Connect failed ret[%d] error[%d]\n", gSID, nErrFromDevice);
        gTunnelStatus = gSID;
    }

    [UIApplication sharedApplication].networkActivityIndicatorVisible = NO;
    return;
}

- (void) StopTunnel
{
    NSLog(@"StopTunnel Start");
    
    if(gPortMappingIndex > 0)
        P2PTunnelAgent_StopPortMapping((unsigned int)gPortMappingIndex);
    
    P2PTunnelAgent_Disconnect(gSID);
    P2PTunnelAgentDeInitialize();
    
    NSLog(@"StopTunnel End");
}

- (int) GetTunnelStatus
{
    return gTunnelStatus;
}

- (int) GetMappedPort
{
    return gMappedPort;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
    //self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    // Override point for customization after application launch.
    //self.window.backgroundColor = [UIColor whiteColor];
    
    [self.window setRootViewController:self.rootController];
    [self.window makeKeyAndVisible];
    
    gTunnelStatus = -40000;
    
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.

    NSLog(@"applicationDidEnterBackground");
    [self StopTunnel];
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
    NSLog(@"applicationDidBecomeActive");
    [self performSelectorInBackground:@selector(StartTunnel) withObject:nil];

}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Saves changes in the application's managed object context before the application terminates.
    [self saveContext];
    [self StopTunnel];
}

- (void)saveContext
{
    NSError *error = nil;
    NSManagedObjectContext *managedObjectContext = self.managedObjectContext;
    if (managedObjectContext != nil) {
        if ([managedObjectContext hasChanges] && ![managedObjectContext save:&error]) {
             // Replace this implementation with code to handle the error appropriately.
             // abort() causes the application to generate a crash log and terminate. You should not use this function in a shipping application, although it may be useful during development. 
            NSLog(@"Unresolved error %@, %@", error, [error userInfo]);
            abort();
        } 
    }
}

#pragma mark - Core Data stack

// Returns the managed object context for the application.
// If the context doesn't already exist, it is created and bound to the persistent store coordinator for the application.
- (NSManagedObjectContext *)managedObjectContext
{
    if (__managedObjectContext != nil) {
        return __managedObjectContext;
    }
    
    NSPersistentStoreCoordinator *coordinator = [self persistentStoreCoordinator];
    if (coordinator != nil) {
        __managedObjectContext = [[NSManagedObjectContext alloc] init];
        [__managedObjectContext setPersistentStoreCoordinator:coordinator];
    }
    return __managedObjectContext;
}

// Returns the managed object model for the application.
// If the model doesn't already exist, it is created from the application's model.
- (NSManagedObjectModel *)managedObjectModel
{
    if (__managedObjectModel != nil) {
        return __managedObjectModel;
    }
    NSURL *modelURL = [[NSBundle mainBundle] URLForResource:@"IOTCBrowser" withExtension:@"momd"];
    __managedObjectModel = [[NSManagedObjectModel alloc] initWithContentsOfURL:modelURL];
    return __managedObjectModel;
}

// Returns the persistent store coordinator for the application.
// If the coordinator doesn't already exist, it is created and the application's store added to it.
- (NSPersistentStoreCoordinator *)persistentStoreCoordinator
{
    if (__persistentStoreCoordinator != nil) {
        return __persistentStoreCoordinator;
    }
    
    NSURL *storeURL = [[self applicationDocumentsDirectory] URLByAppendingPathComponent:@"IOTCBrowser.sqlite"];
    
    NSError *error = nil;
    __persistentStoreCoordinator = [[NSPersistentStoreCoordinator alloc] initWithManagedObjectModel:[self managedObjectModel]];
    if (![__persistentStoreCoordinator addPersistentStoreWithType:NSSQLiteStoreType configuration:nil URL:storeURL options:nil error:&error]) {
        /*
         Replace this implementation with code to handle the error appropriately.
         
         abort() causes the application to generate a crash log and terminate. You should not use this function in a shipping application, although it may be useful during development. 
         
         Typical reasons for an error here include:
         * The persistent store is not accessible;
         * The schema for the persistent store is incompatible with current managed object model.
         Check the error message to determine what the actual problem was.
         
         
         If the persistent store is not accessible, there is typically something wrong with the file path. Often, a file URL is pointing into the application's resources directory instead of a writeable directory.
         
         If you encounter schema incompatibility errors during development, you can reduce their frequency by:
         * Simply deleting the existing store:
         [[NSFileManager defaultManager] removeItemAtURL:storeURL error:nil]
         
         * Performing automatic lightweight migration by passing the following dictionary as the options parameter: 
         [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:YES], NSMigratePersistentStoresAutomaticallyOption, [NSNumber numberWithBool:YES], NSInferMappingModelAutomaticallyOption, nil];
         
         Lightweight migration will only work for a limited set of schema changes; consult "Core Data Model Versioning and Data Migration Programming Guide" for details.
         
         */
        NSLog(@"Unresolved error %@, %@", error, [error userInfo]);
        abort();
    }    
    
    return __persistentStoreCoordinator;
}

#pragma mark - Application's Documents directory

// Returns the URL to the application's Documents directory.
- (NSURL *)applicationDocumentsDirectory
{
    return [[[NSFileManager defaultManager] URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] lastObject];
}

@end
