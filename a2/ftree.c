#include <stdio.h>
// Add your system includes here.
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "ftree.h"


/*
 * Creates the TreeNode for the file/link/directory in the heap
 * Returns a pointer to the heap
 */
struct TreeNode *maker(const char *fnamer, char* path) {
    // Malloc space for TreeNode
    struct TreeNode *nodes = malloc(sizeof(struct TreeNode));
    if (nodes == NULL) {
      fprintf(stderr, "Could not malloc space\n");
      exit(1);
    }

    // Get info about the file
    struct stat file;
    if (lstat(path, &file) == -1) {
      printf("Cant lstat\n");
    }

    // Malloc space for TreeNode->fname char*
    nodes->fname = malloc(sizeof(char*)*strlen(fnamer));
    if (nodes->fname == NULL) {
      fprintf(stderr, "Could not malloc space\n");
      exit(1);
    }

    // Fill in TreeNode data for the File
    strcpy(nodes->fname, fnamer);
    nodes->permissions = file.st_mode & 0777;
    nodes->contents = NULL;
    nodes->next = NULL;
    if (S_ISREG(file.st_mode)) {
      nodes->type = '-';
    } else if (S_ISLNK(file.st_mode)) {
      nodes->type = 'l';
    } else if (S_ISDIR(file.st_mode)) {
      nodes->type = 'd';
    }

    return nodes;
}


/*
 * recursive helper that creates links bettwen TreeNodes
 *
 */
struct TreeNode *make_structs(const char *fnames, char* path) {
    struct stat input;
    struct TreeNode *mains = NULL;
    struct TreeNode *current = NULL;
    struct TreeNode *next = NULL;

    // Create char* of the path to the file, include the filename
    int length = strlen(path) + strlen(fnames) + 2;
    char curr_path[length];
    strcpy(curr_path, path);
    strcat(curr_path, fnames);
    lstat(curr_path, &input);

    // Is a File or Link, create a TreeNode using the current path to it
    // Return that Tree Node. (Base Case)
    if (S_ISREG(input.st_mode)) {
      mains = maker(fnames, curr_path);
      return mains;
    } else if (S_ISLNK(input.st_mode)) {
      mains = maker(fnames, curr_path);
      return mains;
    } else if (S_ISDIR(input.st_mode)) {
      // When it is a directory:
      // Create a TreeNode for the root directory
      mains = maker(fnames, curr_path);

      // Open the Directory
      DIR *directory = opendir(curr_path);
      if (directory == NULL) {
        fprintf(stderr, "Error opening directory: %s\n", curr_path);
        exit(1);
      }

      // Read the directory, first file, filter out an . or ..
      struct dirent *dir_file;
      dir_file = readdir(directory);
      while ((dir_file != NULL)&&((dir_file->d_name)[0] == '.')) {
        dir_file = readdir(directory);
      }

      // Create a TreeNode for first link of file in Directory
      // Assigned to directory contents
      strcat(curr_path, "/");
      mains->contents = make_structs(dir_file->d_name, curr_path);
      current = mains->contents;

      // Read directory for the next file, check if . or ..
      dir_file = readdir(directory);
      while ((dir_file != NULL)&&((dir_file->d_name)[0] == '.')) {
        dir_file = readdir(directory);
      }

      // Loop through directory making TreeNodes for each file,
      // Connect the file TreeNodes as a linked list
      while (dir_file != NULL) {
        next = make_structs(dir_file->d_name, curr_path);
        current->next = next;
        current = next;
        dir_file = readdir(directory);
        while ((dir_file != NULL)&&((dir_file->d_name)[0] == '.')) {
          dir_file = readdir(directory);
        }
      }

      if (closedir(directory) == -1) {
        fprintf(stderr, "Could not close directory\n");
      }
    }
    return mains;
}

/*
 * Returns the FTree rooted at the path fname.
 *
 * Use the following if the file fname doesn't exist and return NULL:
 * fprintf(stderr, "The path (%s) does not point to an existing entry!\n", fname);
 *
 */
struct TreeNode *generate_ftree(const char *fname) {

    // Your implementation here.
    struct stat stat_first;
    if (lstat(fname, &stat_first) == -1) {
        fprintf(stderr, "The path (%s) does not point to an existing entry!\n", fname);
        return NULL;
    }

    struct TreeNode *mains;
    mains = make_structs(fname, "");

    // Hint: consider implementing a recursive helper function that
    // takes fname and a path.  For the initial call on the
    // helper function, the path would be "", since fname is the root
    // of the FTree.  For files at other depths, the path would be the
    // file path from the root to that file.

    return mains;
}


/*
 * Prints the TreeNodes encountered on a preorder traversal of an FTree.
 *
 * The only print statements that you may use in this function are:
 * printf("===== %s (%c%o) =====\n", root->fname, root->type, root->permissions)
 * printf("%s (%c%o)\n", root->fname, root->type, root->permissions)
 *
 */
void print_ftree(struct TreeNode *root) {

    // Here's a trick for remembering what depth (in the tree) you're at
    // and printing 2 * that many spaces at the beginning of the line.
    static int depth = 0;
    // printf("%*s", depth * 2, "");

    // Your implementation here.

    // If file or link, print the root information
    // Check if file/links are connected to more files, via root->next
    if (root->type == '-') {
      printf("%*s", depth * 2, "");
      printf("%s (%c%o)\n", root->fname, root->type, root->permissions);
      if (root->next != NULL) {
        print_ftree(root->next);
      }
    } else if (root->type == 'l') {
      printf("%*s", depth * 2, "");
      printf("%s (%c%o)\n", root->fname, root->type, root->permissions);
      if (root->next != NULL) {
        print_ftree(root->next);
      }
    } else if (root->type == 'd'){
      // When it is a direcotry, check the contents and recursively print
      printf("%*s", depth * 2, "");
      printf("===== %s (%c%o) =====\n", root->fname, root->type, root->permissions);
      // Increase depth space when entering a directory to print contents
      depth++;
      if (root->contents != NULL) {
        print_ftree(root->contents);
      }
      // Decrease depth after finishing printing directory to connect with root
      depth--;
      if (root->next != NULL) {
        print_ftree(root->next);
        root = root->next;
      }
    }
}


/*
 * Deallocate all dynamically-allocated memory in the FTree rooted at node.
 *
 */
void deallocate_ftree (struct TreeNode *node) {

   // Your implementation here.
   // If file or link, free the fname that was malloced and the the root
   // TreeNode that was malloced
   if (node->type == '-') {
     free(node->fname);
     free(node);
   } else if (node->type == 'l'){
     free(node->fname);
     free(node);
   } else {
     // If it is a directory, if contents
     // Similarly by lopping through linked list, loop through file links
     // to free each TreeNode and name
     if (node->contents != NULL) {
       struct TreeNode *curr = node->contents;
       struct TreeNode *root = node->contents;
       while (curr != NULL) {
         if (curr->type == 'd') {
           deallocate_ftree(curr->contents);
         }
         free(curr->fname);
         root = curr->next;
         free(curr);
         curr = root;
       }
       free(node->fname);
       free(node);
     } else {
       // If empty directory
       free(node->fname);
       free(node);
     }
   }

}
