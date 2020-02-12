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
    struct TreeNode *nodes = malloc(sizeof(struct TreeNode));
    if (nodes == NULL) {
      fprintf(stderr, "Could not malloc space\n");
      exit(1);
    }

    struct stat file;
    if (lstat(path, &file) == -1) {
      printf("Cant lstat\n");
    }

    nodes->fname = malloc(sizeof(char*)*strlen(fnamer));
    if (nodes->fname == NULL) {
      fprintf(stderr, "Could not malloc space\n");
      exit(1);
    }
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
    struct TreeNode *main = NULL;
    struct TreeNode *current = NULL;
    struct TreeNode *next = NULL;

    int length = strlen(path) + strlen(fnames) + 2;
    char curr_path[length];
    strcpy(curr_path, path);
    strcat(curr_path, fnames);
    lstat(curr_path, &input);

    // Is a File
    if (S_ISREG(input.st_mode)) {
      main = maker(fnames, curr_path);
      return main;
    } else if (S_ISLNK(input.st_mode)) {
      main = maker(fnames, curr_path);
      return main;
    } else if (S_ISDIR(input.st_mode)) {
      main = maker(fnames, curr_path);
      DIR *directory = opendir(curr_path);
      if (directory == NULL) {
        fprintf(stderr, "Error opening directory: %s\n", curr_path);
        exit(1);
      }
      struct dirent *dir_file;
      dir_file = readdir(directory);
      while ((dir_file != NULL)&&((dir_file->d_name)[0] == '.')) {
        dir_file = readdir(directory);
      }

      strcat(curr_path, "/");
      main->contents = make_structs(dir_file->d_name, curr_path);
      current = main->contents;
      dir_file = readdir(directory);
      while ((dir_file != NULL)&&((dir_file->d_name)[0] == '.')) {
        dir_file = readdir(directory);
      }

      while (dir_file != NULL) {
        next = make_structs(dir_file->d_name, curr_path);
        current->next = next;
        current = next;
        dir_file = readdir(directory);
      }
      closedir(directory);
    }
    return main;
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
      printf("%*s", depth * 2, "");
      printf("===== %s (%c%o) =====\n", root->fname, root->type, root->permissions);
      depth++;
      if (root->contents != NULL) {
        print_ftree(root->contents);
      }
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

}
