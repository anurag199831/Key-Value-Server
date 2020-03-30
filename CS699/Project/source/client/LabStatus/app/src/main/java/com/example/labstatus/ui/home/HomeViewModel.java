package com.example.labstatus.ui.home;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;

public class HomeViewModel extends ViewModel {

    private MutableLiveData<String> mText;



    public String text;

    public String color;

    public HomeViewModel(String t,String c) {
       // mText = new MutableLiveData<>();
        //mText.setValue("This is SL2 fragment");
    text=t;
    color=c;
    }

    public HomeViewModel() {
         mText = new MutableLiveData<>();
        mText.setValue("This is SL2 fragment");

    }

    public LiveData<String> getText() {
        return mText;
    }
}